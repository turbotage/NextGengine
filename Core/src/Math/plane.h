#pragma once

#include "vec3.h"
#include "vec4.h"

namespace ng {
	namespace math {
		class Plane : public Vec4
		{
		private:
			float normOfBasis;
		public:
			Plane(Vec3 point, Vec3 planeVector, Vec3 planeNormal);

			float distanceToPoint(Vec3 point);

		};
	}
}

