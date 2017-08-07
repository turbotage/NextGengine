#pragma once

#include "../math/mat4.h"
#include "entity.h"
#include <vector>
#include "../graphics/vertex.h"

namespace ng {
	namespace entity {
		class Mesh
		{
		private:
			std::vector<ng::graphics::Vertex> vertices;
			std::vector<uint32> indices;
		public:
			Mesh();

			~Mesh();

		};
	}
}

