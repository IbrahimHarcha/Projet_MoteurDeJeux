#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <GL/glew.h>
#include <string>


// Load a .BMP file using our custom loader
GLuint loadBMP_custom(const char * imagepath);

//// Since GLFW 3, glfwLoadTexture2D() has been removed. You have to use another texture loading library, 
//// or do it yourself (just like loadBMP_custom and loadDDS)
//// Load a .TGA file using GLFW's own loader
//GLuint loadTGA_glfw(const char * imagepath);

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(const char * imagepath);

GLuint loadTexture2DFromFilePath(const std::string& path);
GLuint loadTexture2DFromFilePath(const std::string& path, int& width, int& height, int& nrChannels);
unsigned int load_texture_image(std::string file_name, bool& load_complete);


unsigned char* getData(char* path,int &width, int &height, int &nChannels);

#endif