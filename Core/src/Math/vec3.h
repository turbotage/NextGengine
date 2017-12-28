#pragma once

#include <iostream>

namespace ng {
	namespace math {
		class Vec3
		{
		public:
			float x, y, z;

			Vec3();
			Vec3(float scalar);
			Vec3(float x, float y, float z);

			Vec3& add(const Vec3& other);
			Vec3& sub(const Vec3& other);
			Vec3& mul(const Vec3& other);
			Vec3& div(const Vec3& other);

			Vec3& mul(const float& other);
			Vec3& div(const float& other);

			void normalize();
			float norm();

			Vec3 cross(const Vec3& other);

			friend Vec3 operator+(Vec3 left, const Vec3& right);
			friend Vec3 operator-(Vec3 left, const Vec3& right);
			friend Vec3 operator*(Vec3 left, const Vec3& right);
			friend Vec3 operator/(Vec3 left, const Vec3& right);

			friend Vec3 operator*(float left, Vec3 right);
			friend Vec3 operator/(Vec3 left, const float& right);

			bool operator==(const Vec3& other) const;
			bool operator!=(const Vec3& other) const;
			bool operator<(const Vec3& other) const;
			bool operator>(const Vec3& other) const;
			bool operator<=(const Vec3& other) const;
			bool operator>=(const Vec3& other) const;

			Vec3& operator+=(const Vec3& other);
			Vec3& operator-=(const Vec3& other);
			Vec3& operator*=(const Vec3& other);
			Vec3& operator/=(const Vec3& other);

			Vec3& operator+=(float other);
			Vec3& operator-=(float other);
			Vec3& operator*=(float other);
			Vec3& operator/=(float other);

			friend std::ostream& operator<<(std::ostream& stream, const Vec3& vector);
		};
	}
}

namespace std {
	template<> struct hash<ng::math::Vec3> {
		size_t operator()(ng::math::Vec3 const& vector) const {
			return
				hash<float>()(vector.x) ^
				hash<float>()(vector.y) ^
				hash<float>()(vector.z);
		}
	};
}

