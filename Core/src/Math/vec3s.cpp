#include "vec3s.h"

ng::math::Vec3s::Vec3s()
{
}

ng::math::Vec3s::Vec3s(int16 scalar)
	: x(scalar), y(scalar), z(scalar)
{

}

ng::math::Vec3s::Vec3s(int16 x, int16 y, int32 z)
	: x(x), y(y), z(z)
{

}

ng::math::Vec3s & ng::math::Vec3s::add(const Vec3s & other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

ng::math::Vec3s & ng::math::Vec3s::sub(const Vec3s & other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

ng::math::Vec3s & ng::math::Vec3s::mul(const Vec3s & other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	return *this;
}

ng::math::Vec3s & ng::math::Vec3s::div(const Vec3s & other)
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
	return *this;
}

bool ng::math::Vec3s::operator==(const Vec3s other) const
{
	return (x == other.x) && (y == other.y) && (z == other.z);
}

bool ng::math::Vec3s::operator!=(const Vec3s other) const
{
	return (x != other.x) || (y != other.y) || (z != other.z);
}

bool ng::math::Vec3s::operator<(const Vec3s & other) const 
{
	return (x < other.x) && (y < other.y) && (z < other.z);
}

bool ng::math::Vec3s::operator>(const Vec3s & other) const
{
	return (x > other.x) && (y > other.y) && (z > other.z);
}

bool ng::math::Vec3s::operator<=(const Vec3s & other) const
{
	return (x <= other.x) && (y <= other.y) && (z <= other.z);
}

bool ng::math::Vec3s::operator>=(const Vec3s & other) const
{
	return (x >= other.x) && (y >= other.y) && (z >= other.z);
}
