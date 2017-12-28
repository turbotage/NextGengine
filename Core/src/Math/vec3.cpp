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

ng::math::Vec3 & ng::math::Vec3::mul(const float & other)
{
	x *= other;
	y *= other;
	z *= other;
	return *this;
}

ng::math::Vec3 & ng::math::Vec3::div(const float & other)
{
	x /= other;
	y /= other;
	z /= other;
	return *this;
}

void ng::math::Vec3::normalize()
{
	float lenght = sqrt(x*x + y*y + z*z);
	x /= lenght;
	y /= lenght;
	z /= lenght;
}

float ng::math::Vec3::norm()
{
	return sqrt(x*x + y*y + z*z);
}

ng::math::Vec3 ng::math::Vec3::cross(const Vec3 & other)
{
	ng::math::Vec3 ret;
	ret.x = ((y * other.z) - (z * other.y));
	ret.y = -1 * ((x * other.z) - (z * other.x));
	ret.z = ((x * other.y) - (y * other.x));
	return ret;
}

bool ng::math::Vec3::operator==(const Vec3 & other) const
{
	return (x == other.x) && (y == other.y) && (z == other.z);
}

bool ng::math::Vec3::operator!=(const Vec3 & other) const
{
	return (x != other.x) || (y != other.y) || (z != other.z);
}

bool ng::math::Vec3::operator<(const Vec3 & other) const
{
	return (x < other.x) && (y < other.y) && (z < other.z);
}

bool ng::math::Vec3::operator>(const Vec3 & other) const
{
	return (x > other.x) && (y > other.y) && (z > other.z);
}

bool ng::math::Vec3::operator<=(const Vec3 & other) const
{
	return (x <= other.x) && (y <= other.y) && (z <= other.z);
}

bool ng::math::Vec3::operator>=(const Vec3 & other) const
{
	return (x >= other.x) && (y >= other.y) && (z >= other.z);
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

ng::math::Vec3 & ng::math::Vec3::operator+=(float other)
{
	x += other;
	y += other;
	z += other;
	return *this;
}

ng::math::Vec3 & ng::math::Vec3::operator-=(float other)
{
	x -= other;
	y -= other;
	z -= other;
	return *this;
}

ng::math::Vec3 & ng::math::Vec3::operator*=(float other)
{
	x *= other;
	y *= other;
	z *= other;
	return *this;
}

ng::math::Vec3 & ng::math::Vec3::operator/=(float other)
{
	x /= other;
	y /= other;
	z /= other;
	return *this;
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

ng::math::Vec3 ng::math::operator*(float left, Vec3 right)
{
	return right.mul(left);
}

ng::math::Vec3 ng::math::operator/(Vec3 left, const float & right)
{
	return left.div(right);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Vec3 & vector)
{
	stream << "Vec3: (" << vector.x << ", " << vector.y << ", " << vector.z << ")";
	return stream;
}
