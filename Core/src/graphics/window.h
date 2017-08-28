#pragma once

#include "../def.h"
#include <GLFW\glfw3.h>

namespace ng {
	namespace graphics {
		class Window {
			VkInstance* m_Instance;
			VkSurfaceKHR* m_Surface;
		public:

			void init(uint width, uint height, const char* description);

			void createSurface(VkInstance* instance, VkSurfaceKHR* surface);

			void freeSurface();

			void run();
			
			GLFWwindow* glfwWindowPtr;
		};
	}
}


