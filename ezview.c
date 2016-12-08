#define GLFW_DLL 1
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include "ppm\ppm.h"
#include "linmath\linmath.h"
#define M_PI 3.14159265358979323846

// Affine transformation values
float xpos = 0.0;
float ypos = 0.0;
float xtranslate = 0.0;
float ytranslate = 0.0;
float ztranslate = 0.0;
float translation_increment = 2.0;
float scale = 1.0;
float scaling_factor = 1.0;
float rotation = 0.0;
float rotation_increment = 2.0;
float shear_factor = 0.0;


/**
 * This function checks if three integer values against a maximum and minimum values and its primary function 
 * to check color channels.
 *
 * @param red - integer value represents the red color channel
 * @param green - integer value represents the green color channel
 * @param blue - integer value represents the blue color channel
 * @param max - integer value that represents the color channels maximum value
 * @returns min - integer value that represents the color channels maximum value
 */
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


typedef struct Vertex {
	float Position[2];
	float TexCoord[2];
  
} Vertex;


Vertex vertexes[] = {
	{{-1, 1}, {0, 0}},
	{{1, 1}, {1, 0}},

	{{-1, -1}, {0, 1}},
	{{1, 1}, {1, 0}},

	{{1, -1}, {1, 1}},
	{{-1, -1}, {0, 1}}
  
};

static const char* vertex_shader_text =
	"uniform mat4 MVP;\n"
	"attribute vec2 TexCoordIn;\n"
	"attribute vec2 vPos;\n"
	"varying vec2 TexCoordOut;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
	"    TexCoordOut = TexCoordIn;\n"
	"}\n";

static const char* fragment_shader_text =
	"varying lowp vec2 TexCoordOut;\n"
	"uniform sampler2D Texture;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
	"}\n";
	
	
/**
 * error_callback
 *
 * @param
 * @param 
 * @param
 */
static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
	
}


/**
 * key_callback
 *
 * @param TODO
 * @param TODO
 * @param TODO
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {			//<= Close Window
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		
	} else if(key == GLFW_KEY_Q && action == GLFW_PRESS) {     	//<= Scale(Up)
		scale = scale + scaling_factor;
	
	} else if(key == GLFW_KEY_W && action == GLFW_PRESS) {     	//<= Scale(Down)
		scale = scale - scaling_factor;
	
	} else if(key == GLFW_KEY_A && action == GLFW_PRESS) {      //<= Rotate(CCW)
		rotation = rotation + (M_PI/rotation_increment);
	
	} else if(key == GLFW_KEY_S && action == GLFW_PRESS) { 		//<= Rotate(CW)
		rotation = rotation - (M_PI/rotation_increment);
	
	} else if(key == GLFW_KEY_LEFT && action == GLFW_PRESS) { 	//<= Translation(Left)
		xtranslate = xtranslate - translation_increment;
	
	} else if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { 	//<= Translation(Right)
		xtranslate = xtranslate + translation_increment;
	
	} else if(key == GLFW_KEY_UP  && action == GLFW_PRESS) { 	//<= Translation(Up)
		ytranslate = ytranslate + translation_increment;
	
	} else if(key == GLFW_KEY_DOWN  && action == GLFW_PRESS) { 	//<= Translation(Down)
		ytranslate = ytranslate - translation_increment;
	
	} else if(key == GLFW_KEY_Z && action == GLFW_PRESS) { 		//<= Sheer(Left)

	
	} else if(key == GLFW_KEY_X && action == GLFW_PRESS) { 		//<= Sheer(Right)

		
	}
        
}

/**
 * glCompileShaderOrDie
 *
 * @param TODO
 * @param TODO 
 * @param TODO
 */
void glCompileShaderOrDie(GLuint shader) {
	GLint compiled;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	
	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		char* info = malloc(infoLen + 1);
		GLint done;
		glGetShaderInfoLog(shader, infoLen, &done, info);
		printf("Unable to compile shader: %s\n", info);
		exit(1);
	}
	
}


int main(int argc, char *argv[]) {
	Image *ppm_image;
	ppm_image = (Image *)malloc(sizeof(Image));

	// Check number of inputs
	if(argc != 2) {
		fprintf(stderr, "Error, incorrect usage. ezview <input>.ppm\n");
		exit(-1);
	
	// Num inputs pass, proceed to reading in the ppm image file
	} else {
		read_image(argv[1], ppm_image);
		
	}
	
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(ppm_image->width, ppm_image->height, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
		
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShaderOrDie(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShaderOrDie(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
	
    // more error checking! glLinkProgramOrDie!
    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);

    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);

    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 2));

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ppm_image->width, ppm_image->height, 0, GL_RGB, 
		 GL_UNSIGNED_BYTE, ppm_image->image_data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);

    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp, rmat, tmat, smat, shma, arm;
		
		// Setup matrices
		mat4x4_identity(m);
		mat4x4_identity(rmat);
		mat4x4_identity(tmat);
		mat4x4_identity(smat);
		mat4x4_identity(shma);
		mat4x4_identity(arm);

        glfwGetFramebufferSize(window, &width, &height);
        ratio = (float) width / height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
		// Transformation Calculations
		mat4x4_translate(tmat, xtranslate, ytranslate, ztranslate);
		mat4x4_add(m, tmat, m);
		mat4x4_scale_aniso(smat, smat, scale, scale, scale);
		mat4x4_add(m, smat, m);
		mat4x4_rotate_Z(rmat, rmat, rotation);
		mat4x4_mul(m, rmat, m);
		
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
	
	// Deallocate memory previously allocated by calls to malloc
	free(ppm_image->image_data);
	free(ppm_image);
	
    exit(EXIT_SUCCESS);
}
//! [code]
