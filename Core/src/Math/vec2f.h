#pragma once

#include <string>

namespace ng {
	namespace math {
		class Vec2f
		{
		public:
			float x, y;

			Vec2f();
			Vec2f(float scalar);
			Vec2f(float x, float y);

			Vec2f& add(const Vec2f& other);
			Vec2f& sub(const Vec2f& other);
			Vec2f& mul(const Vec2f& other);
			Vec2f& div(const Vec2f& other);

			Vec2f& mul(const float& other);
			Vec2f& div(const float& other);

			friend Vec2f operator+(Vec2f left, const Vec2f& right);
			friend Vec2f operator-(Vec2f left, const Vec2f& right);
			friend Vec2f operator*(Vec2f left, const Vec2f& right);
			friend Vec2f operator/(Vec2f left, const Vec2f& right);

			friend Vec2f operator*(const float& left, Vec2f right);
			friend Vec2f operator/(Vec2f left, const float& rigth);

			bool operator==(const Vec2f& other) const;
			bool operator!=(const Vec2f& other) const;
			bool operator<(const Vec2f& other) const;
			bool operator>(const Vec2f& other) const;
			bool operator<=(const Vec2f& other) const;
			bool operator>=(const Vec2f& other) const;

			Vec2f& operator+=(const Vec2f& other);
			Vec2f& operator-=(const Vec2f& other);
			Vec2f& operator*=(const Vec2f& other);
			Vec2f& operator/=(const Vec2f& other);

			Vec2f& operator+=(float other);
			Vec2f& operator-=(float other);
			Vec2f& operator*=(float other);
			Vec2f& operator/=(float other);

			friend std::ostream& operator<<(std::ostream& stream, const Vec2f& vector);
		};
	}
}

namespace std {
	template<> struct hash<ng::math::Vec2f> {
		size_t operator()(ng::math::Vec2f const& vector) const {
			return
				hash<float>()(vector.x) ^
				hash<float>()(vector.y);
		}
	};
}
