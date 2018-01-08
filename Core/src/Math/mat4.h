#pragma once

#include <smmintrin.h>
#include "../def.h"
#include "Vec3f.h"
#include "Vec4f.h"
#include "quaternion.h"

namespace ng {
	namespace math {
		ALIGN(16) class Mat4
		{
		public:
			ALIGN(16) __m128 rows[4];
			
			Mat4();
			Mat4(float diagonal);
			Mat4(Vec4f diagonal);
			Mat4(Vec4f row1, Vec4f row2, Vec4f row3, Vec4f row4);

			Mat4& mul(const Mat4& other);
			Mat4& operator*=(const Mat4& other);
			friend Mat4 operator*(Mat4 left, const Mat4& right);

			Vec3f mul(const Vec3f& other) const;
			friend Vec3f operator*(const Mat4& left, const Vec3f& right);

			Vec4f mul(const Vec4f& other) const;
			friend Vec4f operator*(const Mat4& left, const Vec4f& right);

			Mat4& add(const Mat4& other);
			Mat4& operator+=(const Mat4& other);
			friend Mat4 operator+(Mat4 left, const Mat4& right);

			Mat4& sub(const Mat4& other);
			Mat4& operator-=(const Mat4& other);
			friend Mat4 operator-(Mat4 left, const Mat4& right);

			Mat4& transpose();

			float det();

			Mat4& invert();

			static Mat4 orthographic(float left, float right, float bottom, float top, float near, float far);
			static Mat4 perspective(float fov, float aspectRatio, float near, float far);
			static Mat4 lookAt(const Vec3f& camera, const Vec3f& object, const Vec3f& up);
			static Mat4 translation(const Vec3f& translation);
			static Mat4 rotation(const Vec3f& axis, float angle);
			static Mat4 rotation(const Quaternion& quat);
			static Mat4 scale(const Vec3f& scale);

			friend std::ostream& operator<<(std::ostream& stream, const Mat4& vector);
		};
	}
}
