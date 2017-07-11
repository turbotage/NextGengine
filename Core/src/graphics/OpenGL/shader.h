#pragma once

#include "graphics.h"

namespace ng {
	namespace graphics {
		class Shader
		{
		public:

			static GLuint getVertexShader(int shaderNumber);
			static GLuint getFragmentShader(int shaderNumber);

			static GLuint createProgram(std::vector<GLuint>& shaders);

			Shader();
		};
	}
}

