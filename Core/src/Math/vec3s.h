#pragma once

#include "../def.h"

namespace ng {
	namespace math {
		class Vec3s
		{
		public:
			int16 x, y, z;

			Vec3s();
			Vec3s(int16 scalar);
			Vec3s(int16 x, int16 y, int32 z);

			Vec3s& add(const Vec3s& other);
			Vec3s& sub(const Vec3s& other);
			Vec3s& mul(const Vec3s& other);
			Vec3s& div(const Vec3s& other);

			bool operator==(const Vec3s other) const;
			bool operator!=(const Vec3s other) const;
			bool operator<(const Vec3s& other) const;
			bool operator>(const Vec3s& other) const;
			bool operator<=(const Vec3s& other) const;
			bool operator>=(const Vec3s& other) const;

		};
	}
}


