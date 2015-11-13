

#include<iostream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <time.h>	

#include <sstream>

#include "glew.h"
#include <GLFW/glfw3.h>
#include "SOIL.h"


GLuint vaoHandle;

int W_WIDTH = 800;
int W_HEIGHT = 600;
//shader private:
GLuint pHandle;
//vertex and fragment shader handle will be used to delete the shader when program ends.
GLuint vertexHandle, fragmentHandle;


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
	//initialize glew, required for every shader program
	GLenum err = glewInit();
	//read Vertex Shader
	//create new shader object, this function return a unique identifier for the shader, which could use to reference the shader
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	vertexHandle = vertShader;

	if (vertShader == 0)
	{
		std::cout << "Error: Cannot create vertex shader";
		exit(1);
	}
	//copy source code into shader object:
	const GLchar * shaderCode = textFileRead(vShaderFile);
	const GLchar* codeArray[] = { shaderCode };
	glShaderSource(vertShader, 1, codeArray, NULL);
	//compile vertex shader:
	glCompileShader(vertShader);
	//verify the shader compilation status:
	GLint result;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		std::cout << "Vertex Shader compilation failed:\n";
		//the code below is to provide the detail of failure:
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
	//=================================end of vertex===============================
	//create, load source and compile fragment shader:
	//read Fragment Shader:
	//create new shader object, this function return a unique identifier for the shader, which could use to reference the shader
	std::cout << "Loading" << fShaderFile << "\n";
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	fragmentHandle = fragShader;

	if (fragShader == 0)
	{
		std::cout << "Error: Cannot create fragment shader";
		exit(1);
	}
	//copy source code into shader object:
	const GLchar * fshaderCode = textFileRead(fShaderFile);
	const GLchar* fcodeArray[] = { fshaderCode };
	glShaderSource(fragShader, 1, fcodeArray, NULL);
	//compile vertex shader:
	glCompileShader(fragShader);
	//verify the shader compilation status:
	GLint fresult;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fresult);
	if (fresult == GL_FALSE)
	{
		std::cout << "Fragment Shader compilation failed:\n";
		//the code below is to provide the detail of failure:
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
	//================================End of fragment============================

	//create program:
	//generate program handle, similar to vertex and fragmentShader handle:
	pHandle = glCreateProgram();
	if (pHandle == 0)
	{
		std::cout << "Error: Cannot create program for shader";
		exit(1);
	}
	//attach shaders to program:
	glAttachShader(pHandle, vertShader);
	glAttachShader(pHandle, fragShader);
	//link the program:
	glLinkProgram(pHandle);
	//after linking the program, we could delete the shader files:
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	//verify link status of program:
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
}
void InitShader()
{
	loadShader("shaders\\NewVert.glsl", "shaders\\NewFrag.glsl");
	//use shader
	glUseProgram(pHandle);
	//send uniform for texture


	float positionData[] =
	{
		-0.8f, -0.8f, 0.0f,
		0.8f, -0.8f, 0.0f,
		0.0f, 0.8f, 0.0f
	};

	float colorData[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
	};

	// create buffer objects
	GLuint vboHandles[2];

	glGenBuffers(2, vboHandles);
	GLuint positionBufferHandle = vboHandles[0];
	GLuint colorBufferHandle = vboHandles[1];

	// populate position buffer

	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), positionData, GL_STATIC_DRAW);

	// populate color buffer
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), colorData, GL_STATIC_DRAW);

	// create and set-up vertex array object
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	// enable vertex attribute arrays
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// map index 0 to position buffer
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

	// map index 1 to color buffer
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);


	//texture:
	const char * filePath = "textures\\brick2.jpg";


	glActiveTexture(GL_TEXTURE0);

	GLuint textureHandles = SOIL_load_OGL_texture
		(
			filePath,
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_INVERT_Y
			| SOIL_FLAG_MIPMAPS);//this function generates a texture buffer object in GPU memory and return a handle to it

								 //if(texture[0] == 0)
								 //return false;
	if (textureHandles <= 0) std::cout << "--Error: can't load " << filePath << "\n"; else
		std::cout << "Done\n";
	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, textureHandles);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	int handle = glGetUniformLocation(pHandle, "Tex1");
	glUniform1i(handle, 0);


}

void init(void)
{	
	GLenum err = glewInit();
	InitShader();
}
//free resource

void display()
{

	glBindVertexArray(vaoHandle);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key <= 254)
	{
		if (action == GLFW_PRESS)
		{
			
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

