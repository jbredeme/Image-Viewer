#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ppm\ppm.h"


GLFWwindow* window;


typedef struct {
	float position[3];
	float color[4];
} Vertex;


const Vertex Vertices[] = {
	{{1, -1, 0}, {1, 0, 0, 1}},
	{{1, 1, 0}, {0, 1, 0, 1}},
	{{-1, 1, 0}, {0, 0, 1, 1}},
	{{-1, -1, 0}, {0, 0, 0, 1}}
};


const GLubyte Indices[] = {
	0, 1, 2,
	2, 3, 0
};


char* vertex_shader_src =
  "attribute vec4 Position;\n"
  "attribute vec4 SourceColor;\n"
  "\n"
  "varying vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    DestinationColor = SourceColor;\n"
  "    gl_Position = Position;\n"
  "}\n";


char* fragment_shader_src =
  "varying lowp vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    gl_FragColor = DestinationColor;\n"
  "}\n";


GLint simple_shader(GLint shader_type, char* shader_src) {

  GLint compile_success = 0;

  int shader_id = glCreateShader(shader_type);

  glShaderSource(shader_id, 1, &shader_src, 0);

  glCompileShader(shader_id);

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);

  if (compile_success == GL_FALSE) {
    GLchar message[256];
    glGetShaderInfoLog(shader_id, sizeof(message), 0, &message[0]);
    printf("glCompileShader Error: %s\n", message);
    exit(1);
  }

  return shader_id;
}


int simple_program() {

  GLint link_success = 0;

  GLint program_id = glCreateProgram();
  GLint vertex_shader = simple_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLint fragment_shader = simple_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

  glAttachShader(program_id, vertex_shader);
  glAttachShader(program_id, fragment_shader);

  glLinkProgram(program_id);

  glGetProgramiv(program_id, GL_LINK_STATUS, &link_success);

  if (link_success == GL_FALSE) {
    GLchar message[256];
    glGetProgramInfoLog(program_id, sizeof(message), 0, &message[0]);
    printf("glLinkProgram Error: %s\n", message);
    exit(1);
  }

  return program_id;
}


static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}


int check_rgb_bits(int red, int green, int blue, int max, int min) {
	if(((red > max) || (red < min)) || ((green > max) || (green < min)) || ((blue > max) || (blue < min))){
		return(1);
		
	} else {
		return(0);
		
	}
		
}


/**
 * Takes in two pointers as parameters, a filename of a ppm image and image structure
 * use to store data read in from the ppm image file.
 *
 * @param filename - string pointer that represents a file name
 * @param image - an image structure 
 */
void read_image(char *filename, Image *image) {
	
    char buffer[64];
	FILE *fpointer;
	int row, column, red, green, blue;
	
	// Open file steam for reading
	fpointer = fopen(filename, "r");
	
	// Check to see if file was opened successfully
	if(fpointer == NULL) {
		fprintf(stderr, "Error, unable to open file.\n");

		// Close file stream flush all buffers
		fclose(fpointer);
		exit(-1);
		 
	} else {
		// Make sure we are reading from the beginning of the file
		rewind(fpointer);

		// Read in the first to characters
		buffer[0] = fgetc(fpointer);
		buffer[1] = fgetc(fpointer);

		// Check the magic number
		if((buffer[0] == 'P') && (buffer[1] == '6')) {
			image->magic_number = "P6";

		} else if((buffer[0] == 'P') && (buffer[1] == '3')) {
			image->magic_number = "P3";

		} else {
			 fprintf(stderr, "Error, unacceptable image format while reading in the file.\n Magic number must be P6 or P3.\n");
			 exit(-2);
			 
		}

		// Ignore comments, whitespaces, carrage returns, and tabs
		while(isdigit(buffer[0]) == 0){
			// If you run into a comment proceed till you reach an newline character
			if(buffer[0] == '#') {
				do {
					buffer[0] = fgetc(fpointer);
					
				} while(buffer[0] != '\n');

			} else {
				buffer[0] = fgetc(fpointer);
				
			}

		}
		
		// Move back one character, tried using 
		ungetc(buffer[0], fpointer);

		// Read in <width> whitespace <height>
		if(fscanf(fpointer, "%d %d", &image->width, &image->height) != 2) {
			 fprintf(stderr, "Error, invalid width and/or height while reading in the file.\n");
			 exit(-2);
			 
		}
		
		// Read in <maximum color value>
		if(fscanf(fpointer, "%d", &image->max_color) != 1) {
			 fprintf(stderr, "Error, invalid maximum color value.\n");
			 exit(-2);
			 
		}

		// Validate 8-bit color value
		if((image->max_color > 255) || (image->max_color < 0)) {
			 fprintf(stderr, "Error, input file's maximum color value is not 8-bits per channel.\n");
			 exit(-2);
			 
		}
		
		// Allocated memory size for image data
		image->image_data = malloc(sizeof(Pixel) * image->width * image->height);

		// If magic number is P6 fread, if magic number is P3 for loop
		if(image->magic_number[1] == '6') {
			// Advance the file pointer by one char
			fgetc(fpointer);
			
			// Read in raw image data
			fread(image->image_data, sizeof(Pixel), image->width * image->height, fpointer);
						
		} else if(image->magic_number[1] == '3') {
			// Read in ascii image data
			for(row = 0; row < image->height; row++) {
				for(column = 0; column < image->width; column++) {					

					// Have to store values into int, larger values more then 1 byte will be truncated
					// therefore making it not possible to check for color channels values over 8-bits	
					fscanf(fpointer, "%d", &red);
					fscanf(fpointer, "%d", &green);
					fscanf(fpointer, "%d", &blue);					
					
					if(check_rgb_bits(red, green, blue, 255, 0) == 1) {
						fprintf(stderr, "Error, a channel color value is not 8-bits.\n");
						exit(-3);
						
					} else {
						image->image_data[(image->width) * row + column].red = red;
						image->image_data[(image->width) * row + column].green = green;
						image->image_data[(image->width) * row + column].blue = blue;
						
					}					
		
				}
				
			}
			
		} else {
			fprintf(stderr, "Error, invalid magic number.\n");
			exit(-2);
			
		}

		// Close file stream flush all buffers
		fclose(fpointer);	
		
	}
	
}


/**
 * main
 * 
 * main function called by the operating system when the user runs the program.
 *
 * @param argc - contains the number of arguments passed to the program
 * @param argv - a one-dimensional array of strings  
 */
int main(int argc, char *argv[]) {
	int count, index;
	FILE *fpointer;
	Image *ppm_image;
	
	// Allocate memory for Image
	ppm_image = (Image *)malloc(sizeof(Image));
	if(ppm_image == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(-1);
		
	}
	
	// Validate command line input(s)
	if(argc != 2){
		fprintf(stderr, "Error, incorrect usage!\nCorrect usage pattern is: ezview input.ppm.\n");
		exit(-1);
		
	} else {
		read_image(argv[1], ppm_image);
		
	}	

	GLint program_id, position_slot, color_slot;
	GLuint vertex_buffer;
	GLuint index_buffer;

	glfwSetErrorCallback(error_callback);

	// Initialize GLFW library
	if (!glfwInit())
	return -1;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create and open a window
	window = glfwCreateWindow(640,
							480,
							"Hello World",
							NULL,
							NULL);

	if (!window) {
	glfwTerminate();
	printf("glfwCreateWindow Error\n");
	exit(1);
	}

	glfwMakeContextCurrent(window);

	program_id = simple_program();

	glUseProgram(program_id);

	position_slot = glGetAttribLocation(program_id, "Position");
	color_slot = glGetAttribLocation(program_id, "SourceColor");
	glEnableVertexAttribArray(position_slot);
	glEnableVertexAttribArray(color_slot);

	// Create Buffer
	glGenBuffers(1, &vertex_buffer);

	// Map GL_ARRAY_BUFFER to this buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// Send the data
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// Repeat
	while (!glfwWindowShouldClose(window)) {

	glClearColor(0, 104.0/255.0, 55.0/255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, 640, 480);

	glVertexAttribPointer(position_slot,
						  3,
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(Vertex),
						  0);

	glVertexAttribPointer(color_slot,
						  4,
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(Vertex),
						  (GLvoid*) (sizeof(float) * 3));

	glDrawElements(GL_TRIANGLES,
				   sizeof(Indices) / sizeof(GLubyte),
				   GL_UNSIGNED_BYTE, 0);

	glfwSwapBuffers(window);
	glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
