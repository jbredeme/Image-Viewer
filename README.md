# Image Viewer ![Jarid_Bredemeier](https://img.shields.io/badge/build-passing-green.svg?style=flat-plastic)
This is an Open GL image viewer that loads PPM images in P3 or P6 format and displays the image in a window. In addition this application  implements keyboard controls to initiate affine transformations on the loaded image.
 
## Screenshots
<img src="https://github.com/jbredeme/Image-Viewer/blob/master/examples/png/original.png" width="288"> <img src="https://github.com/jbredeme/Image-Viewer/blob/master/examples/png/translate.png" width="288"> <img src="https://github.com/jbredeme/Image-Viewer/blob/master/examples/png/scale.png" width="288"><br />
<img src="https://github.com/jbredeme/Image-Viewer/blob/master/examples/png/shear.png" width="288"> <img src="https://github.com/jbredeme/Image-Viewer/blob/master/examples/png/rotate.png" width="288">

## Usage
```c
ezview input.ppm
```
## Interface
<img src="https://github.com/jbredeme/Image-Viewer/blob/master/examples/png/controls.png" width="512">

## Built With
* [GLES2 Starter Kit] for Windows 32-bit
* [Microsoft Visual Studio] 2015 or later
* Windows 10 Professional

## Author
* **Jarid Bredemeier**

## Resources
* [GLFW]
* [Learn OpenGL ES]

[GLES2 Starter Kit]: https://bitbucket.org/jdpalmer/gles2-starter-kit
[GLFW]: http://www.glfw.org/docs/latest/group__keys.html
[Learn OpenGL ES]: http://www.learnopengles.com/tag/linmath-h/
[Microsoft Visual Studio]: https://www.visualstudio.com/downloads/
