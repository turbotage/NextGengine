#pragma once

#include "../def.h"
#include "Vec3f.h"
#include "Vec4f.h"

namespace ng {
	namespace math {
		class Plane : public Vec4f
		{
		private:

		public:

			float normOfBasis;
			ALIGN(16) float a, b, c, d;

			Plane();
			Plane(Vec3f point, Vec3f planeVector, Vec3f planeNormal);

			void setPlane(Vec3f point, Vec3f planeVector, Vec3f planeNormal);

			float distanceToPoint(Vec3f point);

		};
	}
}

