#include <iostream>
#include <stdexcept>
#include "graphics.h"
#include "../debug.h"
#include "shader.h"

ng::graphics::Graphics::Graphics()
{
	
}

ng::graphics::Graphics::Graphics(const char* title, int width, int height)
	: m_Window(title, width, height)
{
	glfwSetErrorCallback(Debug::graphicsErrorCallback);

	if (!glfwInit()) {
		throw std::runtime_error("glfw was unable to init");
	}

	m_Window.init();

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::string str = (const char*)glewGetErrorString(err);
		throw std::runtime_error(str);
	}
	
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glfwSwapInterval(1);

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);
	vertexBuffers.push_back(vertexArrayID);
}

ng::graphics::Graphics::~Graphics()
{
	
}

void ng::graphics::Graphics::run()
{
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	std::vector<GLuint> shaders;
	GLuint vertexShader = Shader::getVertexShader(0);
	GLuint fragmentShader = Shader::getFragmentShader(0);
	shaders.push_back(vertexShader);
	shaders.push_back(fragmentShader);

	program = Shader::createProgram(shaders);

	while (!glfwWindowShouldClose(m_Window.window)) {
		//double time = glfwGetTime();
		//int width, height;
		//glfwGetFramebufferSize(m_Window.window, &width, &height);
		//glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(program);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(0);


		glfwSwapBuffers(m_Window.window);
		glfwPollEvents();
	}
}
