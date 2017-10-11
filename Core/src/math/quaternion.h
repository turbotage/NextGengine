#pragma once

#include "../def.h"
#include <smmintrin.h>
#include "vec3.h"
#include "vec4.h"

namespace ng {
	namespace math {
		class Quaternion
		{
		private:
			float m_MouseSensitivity;

		public:

			union {
				ALIGN(16) float x, y, z, w;
				__m128 row;
			};

			Quaternion();
			Quaternion(float scalar);
			Quaternion(float x, float y, float z, float w);
			Quaternion(const Quaternion& Quaternion);

			Quaternion rotation(const Vec3& rotationAxis, float angle);
			Quaternion rotation(const Vec4& rotationAxis, float angle);

			Vec3 Rotate(const Quaternion& quat, const Vec3& vec);

			~Quaternion();
		};
	}
}
