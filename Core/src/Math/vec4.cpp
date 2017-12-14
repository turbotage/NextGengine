#include "vec4.h"

float ng::math::Vec4::getX() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(0, 0, 0, 0)));
}

float ng::math::Vec4::getY() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(1, 1, 1, 1)));
}

float ng::math::Vec4::getZ() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(2, 2, 2, 2)));
}

float ng::math::Vec4::getW() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(3, 3, 3, 3)));
}

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
	return (getX() == other.getX()) && (getY() == other.getY()) && (getZ() == other.getZ()) && (getW() == other.getW());
}

bool ng::math::Vec4::operator!=(const Vec4 & other) const
{
	return (getX() != other.getX()) || (getY() != other.getY()) || (getZ() != other.getZ()) || (getW() != other.getW());
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
	stream << "Vec4: (" << vector.getX() << ", " << vector.getX() << ", " << vector.getZ() << ", " << vector.getW() << ")";
	return stream;
}
