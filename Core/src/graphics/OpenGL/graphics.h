#pragma once

#include <GL\glew.h>
#define GLFW_DLL
#include <GLFW\glfw3.h>
#include "window.h"
#include <vector>
#include "../../math/mat4.h"

namespace ng {
	namespace graphics {
		class Graphics
		{
			Window m_Window;
		public:

			struct UBO {
				math::Mat4 projection;
				math::Mat4 view;
				math::Mat4 model;
			};

			std::vector<GLuint> vertexBuffers;

			GLuint program;

			GLint mvp_location, vpos_location, vcol_location;
			
			Graphics();
			Graphics(const char* title, int width, int height);
			~Graphics();

			void run();
		};
	}
}

