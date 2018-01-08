#include "Vec4f.h"

float ng::math::Vec4f::getX() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(0, 0, 0, 0)));
}

float ng::math::Vec4f::getY() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(1, 1, 1, 1)));
}

float ng::math::Vec4f::getZ() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(2, 2, 2, 2)));
}

float ng::math::Vec4f::getW() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(3, 3, 3, 3)));
}

ng::math::Vec4f::Vec4f()
{
	_mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
}

ng::math::Vec4f::Vec4f(float scalar)
{
	_mm_set_ps(scalar, scalar, scalar, scalar);
}

ng::math::Vec4f::Vec4f(float x, float y, float z, float w)
{
	_mm_set_ps(w, z, y, x);
}

ng::math::Vec4f & ng::math::Vec4f::add(const Vec4f& other)
{
	row = _mm_add_ps(row, other.row);
	return *this;
}

ng::math::Vec4f & ng::math::Vec4f::sub(const Vec4f & other)
{
	row = _mm_sub_ps(row, other.row);
	return *this;
}

ng::math::Vec4f & ng::math::Vec4f::mul(const Vec4f & other)
{
	row = _mm_mul_ps(row, other.row);
	return *this;
}

ng::math::Vec4f & ng::math::Vec4f::div(const Vec4f & other)
{
	row = _mm_div_ps(row, other.row);
	return *this;
}

bool ng::math::Vec4f::operator==(const Vec4f & other) const
{
	return (getX() == other.getX()) && (getY() == other.getY()) && (getZ() == other.getZ()) && (getW() == other.getW());
}

bool ng::math::Vec4f::operator!=(const Vec4f & other) const
{
	return (getX() != other.getX()) || (getY() != other.getY()) || (getZ() != other.getZ()) || (getW() != other.getW());
}

ng::math::Vec4f & ng::math::Vec4f::operator+=(const Vec4f & other)
{
	return add(other);
}

ng::math::Vec4f & ng::math::Vec4f::operator-=(const Vec4f & other)
{
	return sub(other);
}

ng::math::Vec4f & ng::math::Vec4f::operator*=(const Vec4f & other)
{
	return mul(other);
}

ng::math::Vec4f & ng::math::Vec4f::operator/=(const Vec4f & other)
{
	return div(other);
}

ng::math::Vec4f ng::math::operator+(Vec4f left, const Vec4f & right)
{
	return left.add(right);
}

ng::math::Vec4f ng::math::operator-(Vec4f left, const Vec4f & right)
{
	return left.sub(right);
}

ng::math::Vec4f ng::math::operator*(Vec4f left, const Vec4f & right)
{
	return left.mul(right);
}

ng::math::Vec4f ng::math::operator/(Vec4f left, const Vec4f & right)
{
	return left.div(right);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Vec4f & vector)
{
	stream << "Vec4f: (" << vector.getX() << ", " << vector.getX() << ", " << vector.getZ() << ", " << vector.getW() << ")";
	return stream;
}
