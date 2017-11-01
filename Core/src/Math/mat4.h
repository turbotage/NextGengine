#pragma once

#include <smmintrin.h>
#include "../def.h"
#include "vec3.h"
#include "vec4.h"

namespace ng {
	namespace math {
		class Mat4
		{
		public:
			union {
				ALIGN(16) float elements[16];
				__m128 rows[4];
			};
			
			Mat4();
			Mat4(float diagonal);
			Mat4(Vec4 diagonal);
			Mat4(Vec4 row1, Vec4 row2, Vec4 row3, Vec4 row4);

			Mat4& mul(const Mat4& other);
			Mat4& operator*=(const Mat4& other);
			friend Mat4 operator*(Mat4 left, const Mat4& right);

			Vec3 mul(const Vec3& other) const;
			friend Vec3 operator*(const Mat4& left, const Vec3& right);

			Vec4 mul(const Vec4& other) const;
			friend Vec4 operator*(const Mat4& left, const Vec4& right);

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

			static Mat4 translation(const Vec3& translation);

			static Mat4 rotation(float angle, const Vec3& axis);

			static Mat4 scale(const Vec3& scale);



			friend std::ostream& operator<<(std::ostream& stream, const Mat4& vector);
		};
	}
}
