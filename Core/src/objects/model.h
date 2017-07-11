#pragma once

#include "mesh.h"

namespace ng {
	namespace entity {
		class Model
		{
		private:
			Mesh* m_Mesh;
		public:
			Model();
			~Model();

			void load(const char* path);

		};
	}
}

