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
			
		public:

			union {
				ALIGN(16) float x, y, z, w;
				__m128 row;
			};

			Quaternion();
			Quaternion(float scalar);
			Quaternion(float x, float y, float z, float w);
			Quaternion(const Quaternion& Quaternion);

			Quaternion& add(const Quaternion& other);
			Quaternion& sub(const Quaternion& other);
			Quaternion& mul(const Quaternion& other);

			Quaternion& setRotation(const Vec3& rotationAxis, float angle);
			static Quaternion getRotation(const Vec3& rotationAxis, float angle);
			
			Quaternion& setRotation(const Vec4& rotationAxis, float angle);
			static Quaternion getRotation(const Vec4& rotationAxis, float angle);

			Vec3 Rotate(const Quaternion& quat, const Vec3& vec);
			
			void Rotate4(const Quaternion& quat, Vec3& v1, Vec3& v2, Vec3& v3, Vec3& v4);

			float Norm();
			static float Norm(const Quaternion& quat);

			float Length();
			static float Length(const Quaternion& quat);

			Quaternion& Normalize();
			static Quaternion Normalized(const Quaternion& quat);

			~Quaternion();

		};
	}
}
