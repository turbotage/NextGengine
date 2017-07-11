#include "vec3.h"

ng::math::Vec3::Vec3()
	: x(0), y(0), z(0)
{

}

ng::math::Vec3::Vec3(float scalar)
	: x(scalar), y(scalar), z(scalar)
{
}

ng::math::Vec3::Vec3(float x, float y, float z)
	: x(x), y(y), z(z)
{

}

ng::math::Vec3 & ng::math::Vec3::add(const Vec3 & other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

ng::math::Vec3 & ng::math::Vec3::sub(const Vec3 & other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

ng::math::Vec3 & ng::math::Vec3::mul(const Vec3 & other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	return *this;
}

ng::math::Vec3 & ng::math::Vec3::div(const Vec3 & other)
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
	return *this;
}

bool ng::math::Vec3::operator==(const Vec3 & other) const
{
	return (x == other.x) && (y == other.y) && (z == other.z);
}

bool ng::math::Vec3::operator!=(const Vec3 & other) const
{
	return (x != other.x) || (y != other.y) || (z != other.z);
}

ng::math::Vec3 & ng::math::Vec3::operator+=(const Vec3 & other)
{
	return add(other);
}

ng::math::Vec3 & ng::math::Vec3::operator-=(const Vec3 & other)
{
	return sub(other);
}

ng::math::Vec3 & ng::math::Vec3::operator*=(const Vec3 & other)
{
	return mul(other);;
}

ng::math::Vec3 & ng::math::Vec3::operator/=(const Vec3 & other)
{
	return div(other);
}

ng::math::Vec3 ng::math::operator+(Vec3 left, const Vec3 & right)
{
	return left.add(right);
}

ng::math::Vec3 ng::math::operator-(Vec3 left, const Vec3 & right)
{
	return left.sub(right);
}

ng::math::Vec3 ng::math::operator*(Vec3 left, const Vec3 & right)
{
	return left.mul(right);
}

ng::math::Vec3 ng::math::operator/(Vec3 left, const Vec3 & right)
{
	return left.div(right);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Vec3 & vector)
{
	stream << "Vec3: (" << vector.x << ", " << vector.y << ", " << vector.z << ")";
	return stream;
}
