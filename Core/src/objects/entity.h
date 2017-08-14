#pragma once

#include "../math/mat4.h"

namespace ng {
	namespace entity {
		class Entity
		{
		private:
			math::Mat4 modelView; //orientation of entiry
			math::Mat4 globalView;
			math::Mat4 projectionView;
			//math::Vec3 origin; //position of entity, should be center of mesh, TODO: maybe for a extended version of the class, will take to much memory
		public:

		};
	}
}

