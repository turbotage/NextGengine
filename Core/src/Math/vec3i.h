#pragma once

#include "../def.h"
#include <iostream>

namespace ng {
	namespace math {
		class Vec3i
		{
		public:
			int32 x, y, z;

			Vec3i();
			Vec3i(int32 scalar);
			Vec3i(int32 x, int32 y, int32 z);

			Vec3i& add(const Vec3i& other);
			Vec3i& sub(const Vec3i& other);
			Vec3i& mul(const Vec3i& other);
			Vec3i& div(const Vec3i& other);

			Vec3i& mul(const int32& other);
			Vec3i& div(const int32& other);

			bool operator<(const Vec3i& other) const;
			bool operator>(const Vec3i& other) const;
			bool operator<(const Vec3i& other) const;
			bool operator>(const Vec3i& other) const;
			bool operator<=(const Vec3i& other) const;
			bool operator>=(const Vec3i& other) const;

			Vec3i& operator+=(const Vec3i& other);
			Vec3i& operator-=(const Vec3i& other);
			Vec3i& operator*=(const Vec3i& other);
			Vec3i& operator/=(const Vec3i& other);

			friend std::ostream& operator<<(std::ostream& stream, const Vec3i& vector);
		};
	}
}


