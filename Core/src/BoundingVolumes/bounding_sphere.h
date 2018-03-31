#pragma once

#include "../Math/Vec3f.h"

namespace ng {
	namespace bvolumes {
		class BoundingSphere
		{
		private:

		public:
			ng::math::Vec3f* center;
			float radius;


			BoundingSphere();
			BoundingSphere(ng::math::Vec3f* center, float radius);
		};
	}
}

