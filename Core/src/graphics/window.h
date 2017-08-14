#pragma once

#include "../def.h"
#include <GLFW\glfw3.h>

namespace ng {
	namespace graphics {
		class Window {
		public:

			void init(uint width, uint height, const char* description);

			void run();
			
			GLFWwindow* glfwWindowPtr;
		};
	}
}


