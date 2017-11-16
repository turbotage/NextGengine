#pragma once

#include "mesh.h"

namespace ng {
	namespace entity {
		class Model : Entity
		{
		private:

			Mesh* m_Mesh;

		public:
			bool hasBeenRendered = false;
			bool isRenderable = false;

			void loadPreformated(const char* path);
			void loadUnformated(const char* path);

			void setRenderable(bool )

		};
	}
}

