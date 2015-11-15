#include<iostream>
// glew is needed on Windows because Windows uses the 
// very old openGL 1.x header, so alot of the new shader/ new openGL
// API is not available, for example, without glew, glCreateShader will
// be undefined.
#include "glew.h"
// glfw is like the cross-platform glut. Its manage the window creation
// as well as input handling for an openGL program.
#include <GLFW/glfw3.h>
// SOIL is a texture loading library.
#include "SOIL.h"

int W_WIDTH = 800;
int W_HEIGHT = 600;

///////////////////// For shader //////////////////////
GLuint pHandle;
//vertex and fragment shader handle will be used to delete the shader when program ends.
GLuint vertexHandle, fragmentHandle;
GLuint vaoHandle;

// Hardcoded shader code. Can also load from a file using loadShader
// and the textFileRead function. For using loading from a file, search for where "textFileRead"
// is commented out.

const char * vertexShader =
"#version 330\n"
"layout (location = 0) in vec3 VertexPosition;\n"
"layout(location = 1) in vec2 VertexColor;\n"
"out vec2 Color;\n"
"void main()\n"
"{\n"
"Color = VertexColor;\n"
"gl_Position = vec4(VertexPosition, 1.0);\n"
"}";


const char * pixelShader =
"#version 330\n"
"in vec2 Color;\n"
"uniform sampler2D Tex1;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"vec4 texColor = texture(Tex1, Color);\n"
"FragColor = texColor;\n"
"}";

///////////////////// Loading the bitmap into a texture /////////////////////
// Data read from the header of the BMP file
unsigned char header[54]; // Each BMP file begins by a 54-bytes header
unsigned int dataPos;     // Position in the file where the actual data begins
unsigned int width, height;
unsigned int imageSize;   // = width*height*3
						  // Actual RGB data
unsigned char * data;


//this function just read a file into char array and return
char *textFileRead(const char* shaderFile) {

	FILE* fp = fopen(shaderFile, "rb");
	if (fp == NULL) { return NULL; }
	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	//this is important, the sample file in the book used: buf[size]=' ' which will not work
	buf[size] = '\0';
	fclose(fp);

	return buf;
}


void loadShader(const char* vShaderFile, const char* fShaderFile)
{
	std::cout << "Loading" << vShaderFile << "\n";

	// Read Vertex Shader
	// Create new shader object, this function return a unique identifier for the shader, which could use to reference the shader
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	vertexHandle = vertShader;
	if (vertShader == 0)
	{
		std::cout << "Error: Cannot create vertex shader";
		exit(1);
	}

	// Copy source code into shader object
	const GLchar * shaderCode = vertexShader;
	const GLchar* codeArray[] = { shaderCode };
	glShaderSource(vertShader, 1, codeArray, NULL);
	// Compile vertex shader
	glCompileShader(vertShader);
	// Verify the shader compilation status
	GLint result;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		std::cout << "Vertex Shader compilation failed:\n";
		// Provide the detail of failure:
		GLint logLen;
		glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(vertShader, logLen, &written, log);
			fprintf(stderr, "Shader log:\n%s", log);
			free(log);
		}
	}
	
	
	// Create, load source and compile fragment shader
	// Create new shader object, this function return a unique identifier 
	// for the shader, which could use to reference the shader
	std::cout << "Loading" << fShaderFile << "\n";
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	fragmentHandle = fragShader;
	
	if (fragShader == 0)
	{
		std::cout << "Error: Cannot create fragment shader";
		exit(1);
	}

	// Copy source code into shader object
	const GLchar * fshaderCode = pixelShader; //textFileRead(fShaderFile);
	const GLchar* fcodeArray[] = { fshaderCode };
	glShaderSource(fragShader, 1, fcodeArray, NULL);
	// Compile fragment shader:
	glCompileShader(fragShader);
	// Verify the shader compilation status
	GLint fresult;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fresult);
	if (fresult == GL_FALSE)
	{
		std::cout << "Fragment Shader compilation failed:\n";
		GLint logLen;
		glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(vertShader, logLen, &written, log);
			fprintf(stderr, "Shader log:\n%s", log);
			free(log);
		}
	}
	
	// Create shader program
	pHandle = glCreateProgram();
	if (pHandle == 0)
	{
		std::cout << "Error: Cannot create program for shader";
		exit(1);
	}
	// Attach shaders to program:
	glAttachShader(pHandle, vertShader);
	glAttachShader(pHandle, fragShader);
	// Link the program:
	glLinkProgram(pHandle);
	// After linking the program, we could delete the shader files
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	// Verify link status of program:
	GLint status;
	glGetProgramiv(pHandle, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) //iflink failed then print the failed  status
	{
		fprintf(stderr, "Failed to link shader program!\n");
		GLint logLen;
		glGetProgramiv(pHandle, GL_INFO_LOG_LENGTH,
			&logLen);
		if (logLen > 0)
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(pHandle, logLen,
				&written, log);
			fprintf(stderr, "Program log: \n%s", log);
			free(log);
		}
	}

	// Use the shader program
	glUseProgram(pHandle);
}



GLuint loadBMP(const char * filePath)
{
	// Open the file
	FILE * file = fopen(filePath, "rb");
	if (!file) { printf("Image could not be opened\n"); return 0; }

	//The first thing in the file is a 54-bytes header. It contains information such as
	//“Is this file really a BMP file?”, the size of the image, the number of bits per pixel, etc. So let’s read this header :
	if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return false;
	}

	//The header always begins by BM. 

	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;

	}

	//read the size of the image, the location of the data in the file
	// Read ints from the byte array
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);


	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

										 // Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	//Everything is in memory now, the file can be closed
	fclose(file);
}

// set up VBOs, the VAO to wrap them. VBO store the vertex data in GPU memory buffer
// each buffer store an attribute of the vertex, that would be the input to the vertex shader
// This program has 2 vertex attributes: position and uv texture coordinate. These 2 attributes
// are store in 2 separate buffers on GPU. They are then mapped into the to "in" variable of
// the vertex shader. VAO is just the mapping of which VBO to which attribute( the "in") of
// vertex shader.
void InitVBOAndVAO()
{
	// 2 triangles covering the whole screen space
	float positionData[] =
	{
		1.0f,1.0f,0.0f,		// top right
		1.0f,-1.0f, 0.0f,   // bottom right
		-1.0f, 1.0f, 0.0f,	// top left
		-1.0f, 1.0f, 0.0f,	// top left
		1.0f,-1.0f, 0.0f,	// bottom right
		-1.0f,-1.0f, 0.0f	// bottom left
	};

	// uv coordinate for the 2 triangles above
	float uvData[] =
	{
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	// vertex buffer object to store the vertex position( in screen space, 
	//so no matrix multiplication required)

	// create buffer objects
	GLuint vboHandles[2];

	// first create 2 vbos
	glGenBuffers(2, vboHandles);
	GLuint positionBufferHandle = vboHandles[0];
	GLuint uvCordBufferHandle = vboHandles[1];

	// populate position vbo, 18 elements
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), positionData, GL_STATIC_DRAW);

	// populate uv coordinate vco
	glBindBuffer(GL_ARRAY_BUFFER, uvCordBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), uvData, GL_STATIC_DRAW);

	// create and set-up vertex array object
	// vao is a wrap around all vbo and the attributes they map to, 
	// as input of the vertex shader
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	// enable vertex attribute arrays
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// map index 0 to position buffer
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

	// map index 1 to color buffer
	glBindBuffer(GL_ARRAY_BUFFER, uvCordBufferHandle);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
}

// Example of loading texture, either with SOIL or self creating and loading.
void InitTexture()
{
	////////////////////////////// SELF CREATE and LOAD TEXTURE ///////////////////////
	glActiveTexture(GL_TEXTURE0);
	loadBMP("demo2.bmp");
	// self create a texture and self load bit map file into it
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	////////////////////////////// Auto CREATE AND LOAD TEXTURE WITH SOIL ///////////////////////
	//texture:
	const char * filePath = "textures\\brick2.jpg";
	glActiveTexture(GL_TEXTURE0 + 1);
	GLuint textureHandles = SOIL_load_OGL_texture
		(
			filePath,
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_INVERT_Y
			| SOIL_FLAG_MIPMAPS);//this function generates a texture buffer object in GPU memory and return a handle to it

	if (textureHandles <= 0) std::cout << "--Error: can't load " << filePath << "\n"; else
		std::cout << "Done\n";
	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, textureHandles);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void InitOpenGL()
{
	// right now the 2 file names here is not important since I'm using 
	// hardcoded shader code here. Search for where "textFileRead" is commented out
	// to see how to use the load shader from glsl file.
	loadShader("shaders\\NewVert.glsl", "shaders\\NewFrag.glsl");
	InitVBOAndVAO();
	InitTexture();

	// Set the uniform in fragment shader to use the texture slot 0
	int handle = glGetUniformLocation(pHandle, "Tex1");
	glUniform1i(handle, 0);

}

void init(void)
{	
	GLenum err = glewInit();
	InitOpenGL();
}
//free resource

void display()
{

	glBindVertexArray(vaoHandle);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key <= 254)
	{
		//press E to chnage the texture.
		if (action == GLFW_PRESS && key == GLFW_KEY_E)
		{
			glActiveTexture(GL_TEXTURE0);
			printf("e pressed, load texture now:");
			loadBMP("demo3.bmp");
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 800, 600, GL_BGR, GL_UNSIGNED_BYTE, data);
		}
		else if (action == GLFW_RELEASE)
		{
			
		}

	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	
}

int main(int argc, char **argv)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glViewport(0, 0, W_WIDTH, W_HEIGHT);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	init();
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		
		/* Render here */
		display();
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

