#include "Vec2f.h"

ng::math::Vec2f::Vec2f()
{

}

ng::math::Vec2f::Vec2f(float scalar)
	: x(scalar), y(scalar)
{

}

ng::math::Vec2f::Vec2f(float x, float y)
	: x(x), y(y)
{

}

ng::math::Vec2f & ng::math::Vec2f::add(const Vec2f & other)
{
	x += other.x;
	y += other.y;
	return *this;
}

ng::math::Vec2f & ng::math::Vec2f::sub(const Vec2f & other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

ng::math::Vec2f & ng::math::Vec2f::mul(const Vec2f & other)
{
	x *= other.x;
	y *= other.y;
	return *this;
}

ng::math::Vec2f & ng::math::Vec2f::div(const Vec2f & other)
{
	x /= other.x;
	y /= other.y;
	return *this;
}

ng::math::Vec2f & ng::math::Vec2f::mul(const float & other)
{
	x *= other;
	y *= other;
	return *this;
}

ng::math::Vec2f & ng::math::Vec2f::div(const float & other)
{
	x /= other;
	y /= other;
	return *this;
}

bool ng::math::Vec2f::operator==(const Vec2f & other) const
{
	return (x == other.x) && (y == other.y);
}

bool ng::math::Vec2f::operator!=(const Vec2f & other) const
{
	return (x != other.x) || (y != other.y);
}

bool ng::math::Vec2f::operator<(const Vec2f & other) const
{
	return (x < other.x) && (y < other.y);
}

bool ng::math::Vec2f::operator>(const Vec2f & other) const
{
	return (x > other.x) && (y > other.y);
}

bool ng::math::Vec2f::operator<=(const Vec2f & other) const
{
	return (x <= other.x) && (y <= other.y);
}

bool ng::math::Vec2f::operator>=(const Vec2f & other) const
{
	return (x >= other.x) && (y >= other.y);
}

ng::math::Vec2f & ng::math::Vec2f::operator+=(const Vec2f & other)
{
	return add(other);
}

ng::math::Vec2f & ng::math::Vec2f::operator-=(const Vec2f & other)
{
	return sub(other);
}

ng::math::Vec2f & ng::math::Vec2f::operator*=(const Vec2f & other)
{
	return mul(other);
}

ng::math::Vec2f & ng::math::Vec2f::operator/=(const Vec2f & other)
{
	return div(other);
}

ng::math::Vec2f & ng::math::Vec2f::operator+=(float other)
{
	x += other;
	y += other;
	return *this;
}

ng::math::Vec2f & ng::math::Vec2f::operator-=(float other)
{
	x -= other;
	y -= other;
	return *this;
}

ng::math::Vec2f & ng::math::Vec2f::operator*=(float other)
{
	x *= other;
	y *= other;
	return *this;
}

ng::math::Vec2f & ng::math::Vec2f::operator/=(float other)
{
	x /= other;
	y /= other;
	return *this;
}

ng::math::Vec2f ng::math::operator+(Vec2f left, const Vec2f & right)
{
	return left.add(right);
}

ng::math::Vec2f ng::math::operator-(Vec2f left, const Vec2f & right)
{
	return left.sub(right);
}

ng::math::Vec2f ng::math::operator*(Vec2f left, const Vec2f & right)
{
	return left.mul(right);
}

ng::math::Vec2f ng::math::operator/(Vec2f left, const Vec2f & right)
{
	return left.div(right);
}

ng::math::Vec2f ng::math::operator*(const float & left, Vec2f right)
{
	return right.mul(left);
}

ng::math::Vec2f ng::math::operator/(Vec2f left, const float & rigth)
{
	return left.div(rigth);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Vec2f & vector)
{
	stream << "Vec2f: (" << vector.x << ", " << vector.y << ", " << ")";
	return stream;
}
