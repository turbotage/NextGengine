#pragma once

#include "../def.h"
#include <iostream>
#include <smmintrin.h>

namespace ng {
	namespace math {
		ALIGN(16) class Vec4
		{
		public:

			union {
				ALIGN(16) float x, y, z, w;
				__m128 row;
			};

			Vec4();
			Vec4(float scalar);
			Vec4(float x, float y, float z, float w);

			Vec4& add(const Vec4& other);
			Vec4& sub(const Vec4& other);
			Vec4& mul(const Vec4& other);
			Vec4& div(const Vec4& other);

			friend Vec4 operator+(Vec4 left, const Vec4& right);
			friend Vec4 operator-(Vec4 left, const Vec4& right);
			friend Vec4 operator*(Vec4 left, const Vec4& right);
			friend Vec4 operator/(Vec4 left, const Vec4& right);

			bool operator==(const Vec4& other) const;
			bool operator!=(const Vec4& other) const;

			Vec4& operator+=(const Vec4& other);
			Vec4& operator-=(const Vec4& other);
			Vec4& operator*=(const Vec4& other);
			Vec4& operator/=(const Vec4& other);

			friend std::ostream& operator<<(std::ostream& stream, const Vec4& vector);
		};
	}
}

namespace std {
	template<> struct hash<ng::math::Vec4> {
		size_t operator()(ng::math::Vec4 const& vector) const {
			return
				hash<float>()(vector.x) ^
				hash<float>()(vector.y) ^
				hash<float>()(vector.z) ^
				hash<float>()(vector.w);
		}
	};
}