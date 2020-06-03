#include "vulkan_window.h"

#include "vulkandef.h"

#include <vector>
#include <string>

bool ngv::Window::init() {
	return GLFW_TRUE == glfwInit();
}

void ngv::Window::terminate() {
	glfwTerminate();
}

std::vector<std::string> ngv::Window::getRequiredInstanceExtensions() {
	std::vector<std::string> result;
	uint32 count = 0;
	const char** names = glfwGetRequiredInstanceExtensions(&count);
	if (names && count) {
		for (uint32 i = 0; i < count; ++i) {
			result.emplace_back(names[i]);
		}
	}
	return result;
}

vk::SurfaceKHR ngv::Window::createWindowSurface(GLFWwindow* window, const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator)
{
	VkSurfaceKHR rawSurface;
	vk::Result result = static_cast<vk::Result>(glfwCreateWindowSurface(
		(VkInstance)instance, window, reinterpret_cast<const VkAllocationCallbacks*>(pAllocator), &rawSurface));
	return vk::createResultValue(result, rawSurface, "vk::CommandBuffer::begin");
}

vk::SurfaceKHR ngv::Window::createSurface(const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator)
{
	return createWindowSurface(m_Window, instance, pAllocator);
}

void ngv::Window::swapBuffers() const
{
	glfwSwapBuffers(m_Window);
}

void ngv::Window::createWindow(const glm::uvec2& size, const glm::ivec2& position)
{
	m_Window = glfwCreateWindow(size.x, size.y, "Window Title", nullptr, nullptr);
	if (position != glm::ivec2{ INT_MIN, INT_MIN }) {
		glfwSetWindowPos(m_Window, position.x, position.y);
	}
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetKeyCallback(m_Window, keyboardHandler);
	glfwSetMouseButtonCallback(m_Window, mouseButtonHandler);
	glfwSetCursorPosCallback(m_Window, mouseMoveHandler);
	glfwSetWindowCloseCallback(m_Window, closeHandler);
	glfwSetFramebufferSizeCallback(m_Window, framebufferSizeHandler);
	glfwSetScrollCallback(m_Window, mouseScrollHandler);
}

void ngv::Window::destroyWindow()
{
	glfwDestroyWindow(m_Window);
	m_Window = nullptr;
}

void ngv::Window::makeCurrent() const
{
	glfwMakeContextCurrent(m_Window);
}

void ngv::Window::present() const
{
	glfwSwapBuffers(m_Window);
}

void ngv::Window::showWindow(bool show)
{
	if (show) {
		glfwShowWindow(m_Window);
	}
	else {
		glfwHideWindow(m_Window);
	}
}

void ngv::Window::setTitle(const std::string& title)
{
	glfwSetWindowTitle(m_Window, title.c_str());
}

void ngv::Window::setSizeLimits(const glm::uvec2& minSize, const glm::uvec2& maxSize)
{
	glfwSetWindowSizeLimits(m_Window, minSize.x, minSize.y, (maxSize.x != 0) ? maxSize.x : minSize.x, (maxSize.y != 0) ? maxSize.y : minSize.y);
}

void ngv::Window::runWindowLoop(const std::function<void()>& frameHandler)
{
	while (0 == glfwWindowShouldClose(m_Window)) {
		glfwPollEvents();
		frameHandler();
	}
}

void ngv::Window::onKeyEvent(int key, int scancode, int action, int mods)
{
	switch (action) {
		case GLFW_PRESS:
			onKeyPressed(key, mods);
		case GLFW_RELEASE:
			onKeyReleased(key, mods);
		default:
			break;
	}
}

void ngv::Window::onMouseButtonEvent(int button, int action, int mods)
{
	switch (action) {
		case GLFW_PRESS:
			onMousePressed(button, mods);
			break;
		case GLFW_RELEASE:
			onMouseReleased(button, mods);
			break;
		default:
			break;
	}
}

void ngv::Window::keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Window* example = (Window*)glfwGetWindowUserPointer(window);
	example->onKeyEvent(key, scancode, action, mods);
}

void ngv::Window::mouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{
	Window* example = (Window*)glfwGetWindowUserPointer(window);
	example->onMouseButtonEvent(button, action, mods);
}

void ngv::Window::mouseMoveHandler(GLFWwindow* window, double posx, double posy)
{
	Window* example = (Window*)glfwGetWindowUserPointer(window);
	example->onMouseMoved(glm::vec2(posx, posy));
}

void ngv::Window::mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset)
{
	Window* example = (Window*)glfwGetWindowUserPointer(window);
	example->onMouseScrolled((float)yoffset);
}

void ngv::Window::closeHandler(GLFWwindow* window)
{
	Window* example = (Window*)glfwGetWindowUserPointer(window);
	example->onWindowClosed();
}

void ngv::Window::framebufferSizeHandler(GLFWwindow* window, int width, int height)
{
	Window* example = (Window*)glfwGetWindowUserPointer(window);
	example->onWindowResized(glm::uvec2(width, height));
}
