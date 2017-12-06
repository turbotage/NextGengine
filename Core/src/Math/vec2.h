#pragma once

#include <string>

namespace ng {
	namespace math {
		class Vec2
		{
		public:
			float x, y;

			Vec2();
			Vec2(float scalar);
			Vec2(float x, float y);

			Vec2& add(const Vec2& other);
			Vec2& sub(const Vec2& other);
			Vec2& mul(const Vec2& other);
			Vec2& div(const Vec2& other);

			friend Vec2 operator+(Vec2 left, const Vec2& right);
			friend Vec2 operator-(Vec2 left, const Vec2& right);
			friend Vec2 operator*(Vec2 left, const Vec2& right);
			friend Vec2 operator/(Vec2 left, const Vec2& right);

			bool operator==(const Vec2& other) const;
			bool operator!=(const Vec2& other) const;
			bool operator<(const Vec2& other) const;
			bool operator>(const Vec2& other) const;
			bool operator<=(const Vec2& other) const;
			bool operator>=(const Vec2& other) const;

			Vec2& operator+=(const Vec2& other);
			Vec2& operator-=(const Vec2& other);
			Vec2& operator*=(const Vec2& other);
			Vec2& operator/=(const Vec2& other);

			friend std::ostream& operator<<(std::ostream& stream, const Vec2& vector);
		};
	}
}

namespace std {
	template<> struct hash<ng::math::Vec2> {
		size_t operator()(ng::math::Vec2 const& vector) const {
			return
				hash<float>()(vector.x) ^
				hash<float>()(vector.y);
		}
	};
}
