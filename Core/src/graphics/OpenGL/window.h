#pragma once

#include <GLFW\glfw3.h>

namespace ng {
	namespace graphics {

		class Window
		{
			const char* m_WindowTitle;
			int m_Width, m_Height;
		public:
			GLFWwindow *window;

			Window();
			Window(const char* windowTitle, int width, int height);

			static void graphicsErrorCallback(int error, const char* description);

			static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

			void init();

			void update();

		};

	}
}
