#pragma once

#include "vulkandef.h"
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <functional>

namespace ngv {

	class Window {
	public:

		static bool init();
		static void terminate();

		static std::vector<std::string> getRequiredInstanceExtensions();

		static vk::SurfaceKHR createWindowSurface(
			GLFWwindow* window, const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator = nullptr);

		vk::SurfaceKHR createSurface(const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator = nullptr);

		void swapBuffers() const;
		
		void createWindow(const glm::uvec2& size, const glm::ivec2& position = { INT_MIN, INT_MIN });

		void destroyWindow();

		void makeCurrent() const;

		void present() const;

		void showWindow(bool show = true);

		void setTitle(const std::string& title);

		void setSizeLimits(const glm::uvec2& minSize, const glm::uvec2& maxSize = {});

		void runWindowLoop(const std::function<void()>& frameHandler);

		virtual void onWindowResized(const glm::uvec2& newSize) {};
		virtual void onWindowClosed() {};

		//Keyboard handling
		virtual void onKeyEvent(int key, int scancode, int action, int mods);

		virtual void onKeyPressed(int key, int mods) {}
		virtual void onKeyReleased(int key, int mods) {}

		virtual void onMousePressed(int button, int mods) {}
		virtual void onMouseReleased(int button, int mods) {}
		virtual void onMouseMoved(const glm::vec2& newPos) {}
		virtual void onMouseScrolled(float delta) {}

		//Mouse handling
		virtual void onMouseButtonEvent(int button, int action, int mods);


	private:

		static void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
		static void mouseMoveHandler(GLFWwindow* window, double posx, double posy);
		static void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset);
		static void closeHandler(GLFWwindow* window);
		static void framebufferSizeHandler(GLFWwindow* window, int width, int height);

	private:

		GLFWwindow* m_Window{ nullptr };


	};

}