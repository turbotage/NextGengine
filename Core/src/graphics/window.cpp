#include "window.h"
#include "Vulkan\vulkan_base.h"

void ng::graphics::Window::init(uint width, uint height, const char * description)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowPtr = glfwCreateWindow(width, height, description, nullptr, nullptr);
}

void ng::graphics::Window::createSurface(VkInstance* instance, VkSurfaceKHR* surface)
{
	m_Instance = instance;
	m_Surface = surface;
	if (glfwCreateWindowSurface(*instance, glfwWindowPtr, nullptr, surface)) {
		throw std::runtime_error("failed to create window surface!");
	}


}

void ng::graphics::Window::freeSurface()
{
	vkDestroySurfaceKHR(*m_Instance, *m_Surface, nullptr);
}

void ng::graphics::Window::run()
{
	while (!glfwWindowShouldClose(glfwWindowPtr)) {
		glfwPollEvents();
	}
}

