#pragma once

#include "../def.h"
#include <iostream>
#include <smmintrin.h>

namespace ng {
	namespace math {
		class Vec4f
		{
		private:

		public:

			ALIGN(16) __m128 row;

			float getX() const;
			float getY() const;
			float getZ() const;
			float getW() const;

			Vec4f();
			Vec4f(float scalar);
			Vec4f(float x, float y, float z, float w);

			Vec4f& add(const Vec4f& other);
			Vec4f& sub(const Vec4f& other);
			Vec4f& mul(const Vec4f& other);
			Vec4f& div(const Vec4f& other);

			friend Vec4f operator+(Vec4f left, const Vec4f& right);
			friend Vec4f operator-(Vec4f left, const Vec4f& right);
			friend Vec4f operator*(Vec4f left, const Vec4f& right);
			friend Vec4f operator/(Vec4f left, const Vec4f& right);

			bool operator==(const Vec4f& other) const;
			bool operator!=(const Vec4f& other) const;

			Vec4f& operator+=(const Vec4f& other);
			Vec4f& operator-=(const Vec4f& other);
			Vec4f& operator*=(const Vec4f& other);
			Vec4f& operator/=(const Vec4f& other);

			friend std::ostream& operator<<(std::ostream& stream, const Vec4f& vector);
		
		};
	}
}

namespace std {
	template<> struct hash<ng::math::Vec4f> {
		size_t operator()(ng::math::Vec4f & vector) const {
			return
				hash<float>()(vector.getX()) ^
				hash<float>()(vector.getY()) ^
				hash<float>()(vector.getZ()) ^
				hash<float>()(vector.getW());
		}
	};
}