#pragma once

#include <iostream>

namespace ng {
	namespace math {
		class Vec3f
		{
		public:
			float x, y, z;

			Vec3f();
			Vec3f(float scalar);
			Vec3f(float x, float y, float z);

			Vec3f& add(const Vec3f& other);
			Vec3f& sub(const Vec3f& other);
			Vec3f& mul(const Vec3f& other);
			Vec3f& div(const Vec3f& other);

			Vec3f& mul(const float& other);
			Vec3f& div(const float& other);

			Vec3f& normalize();
			static Vec3f normalized(const Vec3f& other);
			float norm();

			Vec3f& setLength(float length);

			Vec3f cross(const Vec3f& other);

			friend Vec3f operator+(Vec3f left, const Vec3f& right);
			friend Vec3f operator-(Vec3f left, const Vec3f& right);
			friend Vec3f operator*(Vec3f left, const Vec3f& right);
			friend Vec3f operator/(Vec3f left, const Vec3f& right);

			friend Vec3f operator*(const float& left, Vec3f right);
			friend Vec3f operator/(Vec3f left, const float& right);

			bool operator==(const Vec3f& other) const;
			bool operator!=(const Vec3f& other) const;
			bool operator<(const Vec3f& other) const;
			bool operator>(const Vec3f& other) const;
			bool operator<=(const Vec3f& other) const;
			bool operator>=(const Vec3f& other) const;

			Vec3f& operator+=(const Vec3f& other);
			Vec3f& operator-=(const Vec3f& other);
			Vec3f& operator*=(const Vec3f& other);
			Vec3f& operator/=(const Vec3f& other);

			Vec3f& operator+=(float other);
			Vec3f& operator-=(float other);
			Vec3f& operator*=(float other);
			Vec3f& operator/=(float other);

			friend std::ostream& operator<<(std::ostream& stream, const Vec3f& vector);
		};
	}
}

namespace std {
	template<> struct hash<ng::math::Vec3f> {
		size_t operator()(ng::math::Vec3f const& vector) const {
			return
				hash<float>()(vector.x) ^
				hash<float>()(vector.y) ^
				hash<float>()(vector.z);
		}
	};
}

