#pragma once

#include <smmintrin.h>
#include "../def.h"
#include "Vec3f.h"
#include "Vec4f.h"
#include "quaternion.h"

namespace ng {
	namespace math {
		class Mat4f
		{
		public:
			ALIGN(16) __m128 rows[4];

			float getElement(uint8 row, uint8 column);

			Mat4f();
			Mat4f(float diagonal);
			Mat4f(Vec4f diagonal);
			Mat4f(Vec4f row1, Vec4f row2, Vec4f row3, Vec4f row4);

			Mat4f& mul(const Mat4f& other);
			Mat4f& operator*=(const Mat4f& other);
			friend Mat4f operator*(Mat4f left, const Mat4f& right);

			Vec3f mul(const Vec3f& other) const;
			friend Vec3f operator*(const Mat4f& left, const Vec3f& right);

			Vec4f mul(const Vec4f& other) const;
			friend Vec4f operator*(const Mat4f& left, const Vec4f& right);

			Mat4f& add(const Mat4f& other);
			Mat4f& operator+=(const Mat4f& other);
			friend Mat4f operator+(Mat4f left, const Mat4f& right);

			Mat4f& sub(const Mat4f& other);
			Mat4f& operator-=(const Mat4f& other);
			friend Mat4f operator-(Mat4f left, const Mat4f& right);

			Mat4f& transpose();

			float det();

			Mat4f& invert();

			static Mat4f orthographic(float left, float right, float bottom, float top, float near, float far);
			static Mat4f perspective(float fov, float aspectRatio, float near, float far);
			static Mat4f lookAt(const Vec3f& camera, const Vec3f& object, const Vec3f& up);
			static Mat4f translation(const Vec3f& translation);
			static Mat4f rotation(const Vec3f& axis, float angle);
			static Mat4f rotation(const Quaternion& quat);
			static Mat4f scale(const Vec3f& scale);

			friend std::ostream& operator<<(std::ostream& stream, const Mat4f& vector);
		};
	}
}
