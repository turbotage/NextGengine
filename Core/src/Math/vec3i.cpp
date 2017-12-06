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

bool ng::math::Vec3i::operator<(const Vec3i & other)
{
	return (x == other.x) && (y == other.y) && (z == other.z);
}

bool ng::math::Vec3i::operator>(const Vec3i & other)
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
