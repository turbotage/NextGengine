#include "vec3i.h"

ng::math::Vec3i::Vec3i()
{

}

ng::math::Vec3i::Vec3i(int32 scalar)
	: x(scalar), y(scalar), z(scalar)
{

}

ng::math::Vec3i::Vec3i(int32 x, int32 y, int32 z)
	: x(x), y(y), z(z)
{

}

ng::math::Vec3i & ng::math::Vec3i::add(const Vec3i & other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

ng::math::Vec3i & ng::math::Vec3i::sub(const Vec3i & other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

ng::math::Vec3i & ng::math::Vec3i::mul(const Vec3i & other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	return *this;
}

ng::math::Vec3i & ng::math::Vec3i::div(const Vec3i & other)
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
	return *this;
}

ng::math::Vec3i & ng::math::Vec3i::mul(const int32 & other)
{
	x *= other;
	y *= other;
	z *= other;
	return *this;
}

ng::math::Vec3i & ng::math::Vec3i::div(const int32 & other)
{
	x /= other;
	y /= other;
	z /= other;
	return *this;
}

bool ng::math::Vec3i::operator<(const Vec3i & other) const
{
	return (x == other.x) && (y == other.y) && (z == other.z);
}

bool ng::math::Vec3i::operator>(const Vec3i & other) const
{
	return (x == other.x) || (y == other.y) || (z == other.z);
}

bool ng::math::Vec3i::operator<(const Vec3i & other) const
{
	return (x < other.x) && (y < other.y) && (z < other.z);
}

bool ng::math::Vec3i::operator>(const Vec3i & other) const
{
	return (x > other.x) && (y > other.y) && (z > other.z);
}

bool ng::math::Vec3i::operator<=(const Vec3i & other) const
{
	return (x <= other.x) && (y <= other.y) && (z <= other.z);
}

bool ng::math::Vec3i::operator>=(const Vec3i & other) const
{
	return (x >= other.x) && (y >= other.y) && (z >= other.z);
}

ng::math::Vec3i & ng::math::Vec3i::operator+=(const Vec3i & other)
{
	return add(other);
}

ng::math::Vec3i & ng::math::Vec3i::operator-=(const Vec3i & other)
{
	return sub(other);
}

ng::math::Vec3i & ng::math::Vec3i::operator*=(const Vec3i & other)
{
	return mul(other);
}

ng::math::Vec3i & ng::math::Vec3i::operator/=(const Vec3i & other)
{
	return div(other);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Vec3i & vector)
{
	stream << "Vec3i: (" << vector.x << ", " << vector.y << ", " << vector.z << ")";
	return stream;
}
