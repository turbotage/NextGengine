#pragma once

#include "../math/mat4.h"
#include "../graphics/vertex_array.h"
#include "../graphics/index_buffer.h"
#include "../graphics/material.h"
#include <vector>

namespace ng {
	namespace entity {
		class Mesh
		{
		private:
			graphics::VertexArray* m_VertexArray;
			graphics::IndexBuffer* m_IndexBuffer;
			graphics::Material* m_Material;
		public:
			Mesh();
			~Mesh();
		};
	}
}

