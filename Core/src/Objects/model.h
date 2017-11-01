#pragma once

#include "mesh.h"

namespace ng {
	namespace entity {
		class Model : Entity
		{
		private:
			Mesh* m_Mesh;
		public:

			void loadPreformated(const char* path);
			
			void loadUnformated(const char* path);

		};
	}
}

