#include "vec2.h"

ng::math::Vec2::Vec2()
{

}

ng::math::Vec2::Vec2(float scalar)
	: x(scalar), y(scalar)
{

}

ng::math::Vec2::Vec2(float x, float y)
	: x(x), y(y)
{

}

ng::math::Vec2 & ng::math::Vec2::add(const Vec2 & other)
{
	x += other.x;
	y += other.y;
	return *this;
}

ng::math::Vec2 & ng::math::Vec2::sub(const Vec2 & other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

ng::math::Vec2 & ng::math::Vec2::mul(const Vec2 & other)
{
	x *= other.x;
	y *= other.y;
	return *this;
}

ng::math::Vec2 & ng::math::Vec2::div(const Vec2 & other)
{
	x /= other.x;
	y /= other.y;
	return *this;
}

bool ng::math::Vec2::operator==(const Vec2 & other) const
{
	return (x == other.x) && (y == other.y);
}

bool ng::math::Vec2::operator!=(const Vec2 & other) const
{
	return (x != other.x) || (y != other.y);
}

ng::math::Vec2 & ng::math::Vec2::operator+=(const Vec2 & other)
{
	return add(other);
}

ng::math::Vec2 & ng::math::Vec2::operator-=(const Vec2 & other)
{
	return sub(other);
}

ng::math::Vec2 & ng::math::Vec2::operator*=(const Vec2 & other)
{
	return mul(other);
}

ng::math::Vec2 & ng::math::Vec2::operator/=(const Vec2 & other)
{
	return div(other);
}

ng::math::Vec2 ng::math::operator+(Vec2 left, const Vec2 & right)
{
	return left.add(right);
}

ng::math::Vec2 ng::math::operator-(Vec2 left, const Vec2 & right)
{
	return left.sub(right);
}

ng::math::Vec2 ng::math::operator*(Vec2 left, const Vec2 & right)
{
	return left.mul(right);
}

ng::math::Vec2 ng::math::operator/(Vec2 left, const Vec2 & right)
{
	return left.div(right);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Vec2 & vector)
{
	stream << "Vec2: (" << vector.x << ", " << vector.y << ", " << ")";
	return stream;
}
