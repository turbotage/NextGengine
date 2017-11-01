#include "vec4.h"

ng::math::Vec4::Vec4()
{
	_mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
}

ng::math::Vec4::Vec4(float scalar)
{
	_mm_set_ps(scalar, scalar, scalar, scalar);
}

ng::math::Vec4::Vec4(float x, float y, float z, float w)
{
	_mm_set_ps(w, z, y, x);
}

ng::math::Vec4 & ng::math::Vec4::add(const Vec4& other)
{
	row = _mm_add_ps(row, other.row);
	return *this;
}

ng::math::Vec4 & ng::math::Vec4::sub(const Vec4 & other)
{
	row = _mm_sub_ps(row, other.row);
	return *this;
}

ng::math::Vec4 & ng::math::Vec4::mul(const Vec4 & other)
{
	row = _mm_mul_ps(row, other.row);
	return *this;
}

ng::math::Vec4 & ng::math::Vec4::div(const Vec4 & other)
{
	row = _mm_div_ps(row, other.row);
	return *this;
}

bool ng::math::Vec4::operator==(const Vec4 & other) const
{
	return (x == other.x) && (y == other.y) && (z == other.z) && (w == other.w);
}

bool ng::math::Vec4::operator!=(const Vec4 & other) const
{
	return (x != other.x) || (y != other.y) || (z != other.z) || (w != other.w);
}

ng::math::Vec4 & ng::math::Vec4::operator+=(const Vec4 & other)
{
	return add(other);
}

ng::math::Vec4 & ng::math::Vec4::operator-=(const Vec4 & other)
{
	return sub(other);
}

ng::math::Vec4 & ng::math::Vec4::operator*=(const Vec4 & other)
{
	return mul(other);
}

ng::math::Vec4 & ng::math::Vec4::operator/=(const Vec4 & other)
{
	return div(other);
}

ng::math::Vec4 ng::math::operator+(Vec4 left, const Vec4 & right)
{
	return left.add(right);
}

ng::math::Vec4 ng::math::operator-(Vec4 left, const Vec4 & right)
{
	return left.sub(right);
}

ng::math::Vec4 ng::math::operator*(Vec4 left, const Vec4 & right)
{
	return left.mul(right);
}

ng::math::Vec4 ng::math::operator/(Vec4 left, const Vec4 & right)
{
	return left.div(right);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Vec4 & vector)
{
	stream << "Vec4: (" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
	return stream;
}
