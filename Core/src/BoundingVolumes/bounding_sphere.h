#pragma once

#include "vec3f.h"

namespace ng {
	namespace bvolumes {
		class BoundingSphere
		{
		private:

		public:
			ngm::Vec3f* center;
			float radius;


			BoundingSphere();
			BoundingSphere(ngm::Vec3f* center, float radius);
		};
	}
}

