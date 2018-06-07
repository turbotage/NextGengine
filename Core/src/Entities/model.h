#pragma once

#include "mesh.h"
#include "../BoundingVolumes/general_bv.h"

namespace ng {
	namespace entity {
		class Model : Entity
		{
		private:
			Mesh* Mesh;

		public:
			bool hasBeenRendered = false;
			bool isRenderable = false;

			void loadPreformated(const char* path);
			void loadUnformated(const char* path);

			void setRenderable(bool renderable);

		};
	}
}

