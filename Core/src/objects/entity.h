#pragma once

#include "../math/mat4.h"

namespace ng {
	namespace entity {
		class Entity
		{
		private:
			math::Mat4 orientation; //orientation of entiry
			math::Vec3 origin; //position of entity, should be center of mesh
		public:

			Entity();
			~Entity();

		};
	}
}

