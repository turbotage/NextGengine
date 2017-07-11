#include "window.h"
#include <stdexcept>

ng::graphics::Window::Window()
{
}

ng::graphics::Window::Window(const char * windowTitle, int width, int height)
	: m_WindowTitle(windowTitle), m_Width(width), m_Height(height)
{

}

void ng::graphics::Window::graphicsErrorCallback(int error, const char * description)
{
	throw std::runtime_error(description);
}

void ng::graphics::Window::key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	//The key callback
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void ng::graphics::Window::init()
{
	window = glfwCreateWindow(m_Width, m_Height, m_WindowTitle, NULL, NULL);
	if (!window) {
		glfwTerminate();
		throw std::runtime_error("failed to create window");
	}
	glfwMakeContextCurrent(window);
}

void ng::graphics::Window::update()
{
}

