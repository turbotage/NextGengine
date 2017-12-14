#pragma once

#include "../def.h"
#include <smmintrin.h>
#include "vec3.h"
#include "vec4.h"

namespace ng {
	namespace math {
		ALIGN(16) class Quaternion
		{
		private:

		public:

			__m128 row;

			float getX() const;
			float getY() const;
			float getZ() const;
			float getW() const;

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

			Vec3 EulerAngles();
			static Vec3 getEulerAngles(const Quaternion& quat);

			Vec3 Rotate(const Vec3& vec);
			static Vec3 getRotation(const Quaternion& quat, const Vec3& vec);

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
