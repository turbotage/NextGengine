#include "mat4.h"
#include "../def.h"

#define _mm_shufd(xmm, mask) _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(xmm), mask))

ng::math::Mat4::Mat4()
{
}

ng::math::Mat4::Mat4(float diagonal)
{
	rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, diagonal);
	rows[1] = _mm_set_ps(0.0f, 0.0f, diagonal, 0.0f);
	rows[2] = _mm_set_ps(0.0f, diagonal, 0.0f, 0.0f);
	rows[3] = _mm_set_ps(diagonal, 0.0f, 0.0f, 0.0f);
}

ng::math::Mat4::Mat4(Vec4 diagonal)
{
	rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, diagonal.x);
	rows[1] = _mm_set_ps(0.0f, 0.0f, diagonal.y, 0.0f);
	rows[2] = _mm_set_ps(0.0f, diagonal.z, 0.0f, 0.0f);
	rows[3] = _mm_set_ps(diagonal.w, 0.0f, 0.0f, 0.0f);
}

ng::math::Mat4::Mat4(Vec4 row1, Vec4 row2, Vec4 row3, Vec4 row4)
{
	rows[0] = _mm_set_ps(row1.w, row1.z, row1.y, row1.x);
	rows[1] = _mm_set_ps(row2.w, row2.z, row2.y, row2.x);
	rows[2] = _mm_set_ps(row3.w, row3.z, row3.y, row3.x);
	rows[3] = _mm_set_ps(row4.w, row4.z, row4.y, row4.x);
}

ng::math::Mat4 & ng::math::Mat4::mul(const Mat4 & other)
{
	for (int i = 0; i < 4; ++i) {
		__m128 brod1 = _mm_set1_ps(rows[i].m128_f32[0]);
		__m128 brod2 = _mm_set1_ps(rows[i].m128_f32[1]);
		__m128 brod3 = _mm_set1_ps(rows[i].m128_f32[2]);
		__m128 brod4 = _mm_set1_ps(rows[i].m128_f32[3]);
		rows[i] = _mm_add_ps(
			_mm_add_ps(
				_mm_mul_ps(brod1, other.rows[0]),
				_mm_mul_ps(brod2, other.rows[1])),
			_mm_add_ps(
				_mm_mul_ps(brod3, other.rows[2]),
				_mm_mul_ps(brod4, other.rows[3])));
	}
	return *this;
}

ng::math::Mat4 & ng::math::Mat4::operator*=(const Mat4 & other)
{
	return mul(other);
}

ng::math::Vec3 ng::math::Mat4::mul(const Vec3 & other) const
{
	__m128 row = _mm_set_ps(1.0f, other.z, other.y, other.x);
	__m128 ret = _mm_add_ps(
				_mm_add_ps(
					_mm_dp_ps(rows[0], row, 0b11110001), 
					_mm_dp_ps(rows[1], row, 0b11110010)
				),
				_mm_add_ps(
					_mm_dp_ps(rows[2], row, 0b11110100),
					_mm_dp_ps(rows[3], row, 0b11111000)
				)
			);
	return Vec3(ret.m128_f32[0], ret.m128_f32[1], ret.m128_f32[2]);
}

ng::math::Vec4 ng::math::Mat4::mul(const Vec4 & other) const
{
	__m128 row = _mm_load_ps(&other.x);
	__m128 ret = _mm_add_ps(
		_mm_add_ps(
			_mm_dp_ps(rows[0], row, 0b11110001),
			_mm_dp_ps(rows[1], row, 0b11110010)
		),
		_mm_add_ps(
			_mm_dp_ps(rows[2], row, 0b11110100),
			_mm_dp_ps(rows[3], row, 0b11111000)
		)
	);
	Vec4 returner;
	_mm_store_ps(&returner.x, ret);
	return returner;
}

ng::math::Mat4 & ng::math::Mat4::add(const Mat4 & other)
{
	rows[0] = _mm_add_ps(rows[0], other.rows[0]);
	rows[1] = _mm_add_ps(rows[1], other.rows[1]);
	rows[2] = _mm_add_ps(rows[2], other.rows[2]);
	rows[3] = _mm_add_ps(rows[3], other.rows[3]);
	return *this;
}

ng::math::Mat4 & ng::math::Mat4::sub(const Mat4 & other)
{
	rows[0] = _mm_sub_ps(rows[0], other.rows[0]);
	rows[1] = _mm_sub_ps(rows[1], other.rows[1]);
	rows[2] = _mm_sub_ps(rows[2], other.rows[2]);
	rows[3] = _mm_sub_ps(rows[3], other.rows[3]);
	return *this;
}

ng::math::Mat4 & ng::math::Mat4::operator-=(const Mat4 & other)
{
	return sub(other);
}

ng::math::Mat4 & ng::math::Mat4::operator+=(const Mat4 & other)
{
	return add(other);
}

ng::math::Mat4 & ng::math::Mat4::transpose()
{
	__m128 t1 = _mm_unpacklo_ps(rows[0], rows[1]);
	__m128 t2 = _mm_unpacklo_ps(rows[2], rows[3]);
	__m128 t3 = _mm_unpackhi_ps(rows[0], rows[1]);
	__m128 t4 = _mm_unpackhi_ps(rows[2], rows[3]);
	rows[0] = _mm_movelh_ps(t1, t2);
	rows[1] = _mm_movehl_ps(t2, t1);
	rows[2] = _mm_movelh_ps(t3, t4);
	rows[3] = _mm_movehl_ps(t4, t3);
	return *this;
}

float ng::math::Mat4::det()
{
	__m128 r = _mm_shufd(rows[2], 0x39);
	__m128 v1 = _mm_mul_ps(r, rows[3]);
	__m128 v2 = _mm_mul_ps(r, _mm_shufd(rows[3], 0x4E));
	__m128 v3 = _mm_mul_ps(r, _mm_shufd(rows[3], 0x93));
	__m128 r1 = _mm_sub_ps(_mm_shufd(v2, 0x39),
		_mm_shufd(v1, 0x4E));
	__m128 r2 = _mm_sub_ps(_mm_shufd(v3, 0x4E), v3);
	__m128 r3 = _mm_sub_ps(v2, _mm_shufd(v1, 0x39));

	v1 = _mm_shufd(rows[1], 0x93);
	v2 = _mm_shufd(rows[1], 0x39);
	v3 = _mm_shufd(rows[1], 0x4E);
	__m128 d = _mm_mul_ps(_mm_add_ps(_mm_add_ps(
		_mm_mul_ps(v2, r1),
		_mm_mul_ps(v3, r2)),
		_mm_mul_ps(v1, r3)), rows[0]);
	d = _mm_add_ps(d, _mm_shufd(d, 0x4E));
	d = _mm_sub_ss(d, _mm_shufd(d, 0x11));
	return _mm_cvtss_f32(d);
}

ng::math::Mat4 & ng::math::Mat4::invert()
{

	__m128 f1 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(rows[2], rows[1], 0xAA),
		_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0xFF), 0x80)),
		_mm_mul_ps(_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0xAA), 0x80),
			_mm_shuffle_ps(rows[2], rows[1], 0xFF)));
	__m128 f2 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(rows[2], rows[1], 0x55),
		_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0xFF), 0x80)),
		_mm_mul_ps(_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0x55), 0x80),
			_mm_shuffle_ps(rows[2], rows[1], 0xFF)));
	__m128 f3 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(rows[2], rows[1], 0x55),
		_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0xAA), 0x80)),
		_mm_mul_ps(_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0x55), 0x80),
			_mm_shuffle_ps(rows[2], rows[1], 0xAA)));
	__m128 f4 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(rows[2], rows[1], 0x00),
		_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0xFF), 0x80)),
		_mm_mul_ps(_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0x00), 0x80),
			_mm_shuffle_ps(rows[2], rows[1], 0xFF)));
	__m128 f5 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(rows[2], rows[1], 0x00),
		_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0xAA), 0x80)),
		_mm_mul_ps(_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0x00), 0x80),
			_mm_shuffle_ps(rows[2], rows[1], 0xAA)));
	__m128 f6 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(rows[2], rows[1], 0x00),
		_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0x55), 0x80)),
		_mm_mul_ps(_mm_shufd(_mm_shuffle_ps(rows[3], rows[2], 0x00), 0x80),
			_mm_shuffle_ps(rows[2], rows[1], 0x55)));
	__m128 v1 = _mm_shufd(_mm_shuffle_ps(rows[1], rows[0], 0x00), 0xA8);
	__m128 v2 = _mm_shufd(_mm_shuffle_ps(rows[1], rows[0], 0x55), 0xA8);
	__m128 v3 = _mm_shufd(_mm_shuffle_ps(rows[1], rows[0], 0xAA), 0xA8);
	__m128 v4 = _mm_shufd(_mm_shuffle_ps(rows[1], rows[0], 0xFF), 0xA8);
	__m128 s1 = _mm_set_ps(-0.0f, 0.0f, -0.0f, 0.0f);
	__m128 s2 = _mm_set_ps(0.0f, -0.0f, 0.0f, -0.0f);
	__m128 i1 = _mm_xor_ps(s1, _mm_add_ps(
		_mm_sub_ps(_mm_mul_ps(v2, f1),
			_mm_mul_ps(v3, f2)),
		_mm_mul_ps(v4, f3)));
	__m128 i2 = _mm_xor_ps(s2, _mm_add_ps(
		_mm_sub_ps(_mm_mul_ps(v1, f1),
			_mm_mul_ps(v3, f4)),
		_mm_mul_ps(v4, f5)));
	__m128 i3 = _mm_xor_ps(s1, _mm_add_ps(
		_mm_sub_ps(_mm_mul_ps(v1, f2),
			_mm_mul_ps(v2, f4)),
		_mm_mul_ps(v4, f6)));
	__m128 i4 = _mm_xor_ps(s2, _mm_add_ps(
		_mm_sub_ps(_mm_mul_ps(v1, f3),
			_mm_mul_ps(v2, f5)),
		_mm_mul_ps(v3, f6)));
	__m128 d = _mm_mul_ps(rows[0], _mm_movelh_ps(_mm_unpacklo_ps(i1, i2),
		_mm_unpacklo_ps(i3, i4)));
	d = _mm_add_ps(d, _mm_shufd(d, 0x4E));
	d = _mm_add_ps(d, _mm_shufd(d, 0x11));
	d = _mm_div_ps(_mm_set1_ps(1.0f), d);

	rows[0] = _mm_mul_ps(i1, d);
	rows[1] = _mm_mul_ps(i2, d);
	rows[2] = _mm_mul_ps(i3, d);
	rows[3] = _mm_mul_ps(i4, d);
	return *this;
}

ng::math::Mat4 ng::math::Mat4::orthographic(float left, float right, float bottom, float top, float near, float far)
{
	Mat4 returner;
	returner.rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, 2.0f / (right - left));
	returner.rows[1] = _mm_set_ps(0.0f, 0.0f, 2.0f / (top - bottom), 0.0f);
	returner.rows[2] = _mm_set_ps(0.0f, 2.0f / (near - far), 0.0f, 0.0f);
	returner.rows[3] = _mm_set_ps(1.0f, (far + near) / (far - near), (bottom + top) / (bottom - top), (left + right) / (left - right));
	return returner;
}

ng::math::Mat4 ng::math::Mat4::perspective(float fov, float aspectRatio, float near, float far)
{
	Mat4 returner;
	float q = 1.0f / tan(0.5 * fov);
	float a = q / aspectRatio;
	float b = (near + far) / (near - far);
	float c = (2.0f * near * far) / (near - far);
	returner.rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, a);
	returner.rows[1] = _mm_set_ps(0.0f, 0.0f, q, 0.0f);
	returner.rows[2] = _mm_set_ps(-1.0f, b, 0.0f, 0.0f);
	returner.rows[3] = _mm_set_ps(0.0f, c, 0.0f, 0.0f);
	return returner;
}

ng::math::Mat4 ng::math::Mat4::translation(const Vec3 & translation)
{
	Mat4 returner;
	returner.rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
	returner.rows[1] = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	returner.rows[2] = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	returner.rows[3] = _mm_set_ps(1.0f, translation.z, translation.y, translation.x);
	return returner;
}

ng::math::Mat4 ng::math::Mat4::rotation(float angle, const Vec3& axis)
{
#define x axis.x
#define y axis.y
#define z axis.z

	Mat4 returner;
	float c = cos(angle);
	float s = sin(angle);
	float omc = 1.0f - c;
	returner.rows[0] = _mm_set_ps(
		0.0f, 
		x * z * omc + y * s,
		x * y * omc - z * s,
		x * x * omc + c
	);
	returner.rows[1] = _mm_set_ps(
		0.0f, 
		y * z * omc - x * s,
		y * y * omc + c,
		y * x * omc + z * s
	);
	returner.rows[2] = _mm_set_ps(
		0.0f, 
		z * z * omc + c,
		y * z * omc + x * s,
		x * z * omc - y * s
	);
	returner.rows[3] = _mm_set_ps(
		1.0f, 
		0.0f, 
		0.0f, 
		0.0f
	);
	return returner;
#undef x
#undef y
#undef z
}

ng::math::Mat4 ng::math::Mat4::scale(const Vec3 & scale)
{
	Mat4 returner;
	returner.rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, scale.x);
	returner.rows[1] = _mm_set_ps(0.0f, 0.0f, scale.y, 0.0f);
	returner.rows[2] = _mm_set_ps(0.0f, scale.z, 0.0f, 0.0f);
	returner.rows[3] = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	return returner;
}

ng::math::Mat4 ng::math::operator*(Mat4 left, const Mat4 & right)
{
	return left.mul(right);
}

ng::math::Vec3 ng::math::operator*(const Mat4 & left, const Vec3 & right)
{
	return left.mul(right);
}

ng::math::Vec4 ng::math::operator*(const Mat4 & left, const Vec4 & right)
{
	return left.mul(right);
}

ng::math::Mat4 ng::math::operator+(Mat4 left, const Mat4 & right)
{
	return left.add(right);
}

ng::math::Mat4 ng::math::operator-(Mat4 left, const Mat4 & right)
{
	return left.sub(right);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Mat4 & matrix)
{
	stream << "Mat4: [\n";
	for (int i = 0; i < 4; ++i) {
		stream << matrix.rows[i].m128_f32[0] << ", " << matrix.rows[i].m128_f32[1] << ", " << matrix.rows[i].m128_f32[2] << ", " << matrix.rows[i].m128_f32[3] << std::endl;
	}
	return stream;
}





#undef _mm_shufd