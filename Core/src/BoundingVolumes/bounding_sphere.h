#pragma once

#include "../Math/Vec3f.h"

namespace ng {
	namespace bvolumes {
		class BoundingSphere
		{
		private:

		public:
			ng::math::Vec3f centerPos;
			float radius;


			BoundingSphere();
			BoundingSphere(ng::math::Vec3f pos, float radius);
		};
	}
}

