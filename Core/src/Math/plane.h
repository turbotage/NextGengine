#pragma once

#include "../def.h"
#include "vec3.h"
#include "vec4.h"

namespace ng {
	namespace math {
		class Plane : public Vec4
		{
		private:

		public:

			float normOfBasis;
			ALIGN(16) float a, b, c, d;

			Plane();
			Plane(Vec3 point, Vec3 planeVector, Vec3 planeNormal);

			void setPlane(Vec3 point, Vec3 planeVector, Vec3 planeNormal);

			float distanceToPoint(Vec3 point);

		};
	}
}

