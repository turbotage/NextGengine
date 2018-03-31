#include "mat4f.h"
#include "../def.h"
#include <random>

#define _mm_shufd(xmm, mask) _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(xmm), mask))

float ng::math::Mat4f::getElement(uint8 row, uint8 column)
{
	return _mm_cvtss_f32(_mm_shuffle_ps(rows[row], rows[row], _MM_SHUFFLE(column - 1, column - 1, column - 1, column - 1)));
}

ng::math::Mat4f::Mat4f()
{
}

ng::math::Mat4f::Mat4f(float diagonal)
{
	rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, diagonal);
	rows[1] = _mm_set_ps(0.0f, 0.0f, diagonal, 0.0f);
	rows[2] = _mm_set_ps(0.0f, diagonal, 0.0f, 0.0f);
	rows[3] = _mm_set_ps(diagonal, 0.0f, 0.0f, 0.0f);
}

ng::math::Mat4f::Mat4f(Vec4f diagonal)
{
	rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, diagonal.getX());
	rows[1] = _mm_set_ps(0.0f, 0.0f, diagonal.getY(), 0.0f);
	rows[2] = _mm_set_ps(0.0f, diagonal.getZ(), 0.0f, 0.0f);
	rows[3] = _mm_set_ps(diagonal.getW(), 0.0f, 0.0f, 0.0f);
}

ng::math::Mat4f::Mat4f(Vec4f row1, Vec4f row2, Vec4f row3, Vec4f row4)
{
	rows[0] = _mm_set_ps(row1.getW(), row1.getZ(), row1.getY(), row1.getX());
	rows[1] = _mm_set_ps(row2.getW(), row2.getZ(), row2.getY(), row2.getX());
	rows[2] = _mm_set_ps(row3.getW(), row3.getZ(), row3.getY(), row3.getX());
	rows[3] = _mm_set_ps(row4.getW(), row4.getZ(), row4.getY(), row4.getX());
}

ng::math::Mat4f & ng::math::Mat4f::mul(const Mat4f & other)
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

ng::math::Mat4f & ng::math::Mat4f::operator*=(const Mat4f & other)
{
	return mul(other);
}

ng::math::Vec3f ng::math::Mat4f::mul(const Vec3f & other) const
{
	Vec4f ret;
	ret.row = _mm_set_ps(1.0f, other.z, other.y, other.x);
	ret.row = _mm_add_ps(
				_mm_add_ps(
					_mm_dp_ps(rows[0], ret.row, 0b11110001),
					_mm_dp_ps(rows[1], ret.row, 0b11110010)
				),
				_mm_add_ps(
					_mm_dp_ps(rows[2], ret.row, 0b11110100),
					_mm_dp_ps(rows[3], ret.row, 0b11111000)
				)
			);
	return Vec3f(ret.getX(), ret.getY(), ret.getZ());
}

ng::math::Vec4f ng::math::Mat4f::mul(const Vec4f & other) const
{
	Vec4f returner;
	returner.row = _mm_add_ps(
		_mm_add_ps(
			_mm_dp_ps(rows[0], other.row, 0b11110001),
			_mm_dp_ps(rows[1], other.row, 0b11110010)
		),
		_mm_add_ps(
			_mm_dp_ps(rows[2], other.row, 0b11110100),
			_mm_dp_ps(rows[3], other.row, 0b11111000)
		)
	);
	return returner;
}

ng::math::Mat4f & ng::math::Mat4f::add(const Mat4f & other)
{
	rows[0] = _mm_add_ps(rows[0], other.rows[0]);
	rows[1] = _mm_add_ps(rows[1], other.rows[1]);
	rows[2] = _mm_add_ps(rows[2], other.rows[2]);
	rows[3] = _mm_add_ps(rows[3], other.rows[3]);
	return *this;
}

ng::math::Mat4f & ng::math::Mat4f::sub(const Mat4f & other)
{
	rows[0] = _mm_sub_ps(rows[0], other.rows[0]);
	rows[1] = _mm_sub_ps(rows[1], other.rows[1]);
	rows[2] = _mm_sub_ps(rows[2], other.rows[2]);
	rows[3] = _mm_sub_ps(rows[3], other.rows[3]);
	return *this;
}

ng::math::Mat4f & ng::math::Mat4f::operator-=(const Mat4f & other)
{
	return sub(other);
}

ng::math::Mat4f & ng::math::Mat4f::operator+=(const Mat4f & other)
{
	return add(other);
}

ng::math::Mat4f & ng::math::Mat4f::transpose()
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

float ng::math::Mat4f::det()
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

ng::math::Mat4f & ng::math::Mat4f::invert()
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

ng::math::Mat4f ng::math::Mat4f::orthographic(float left, float right, float bottom, float top, float near, float far)
{
	Mat4f returner;
	returner.rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, 2.0f / (right - left));
	returner.rows[1] = _mm_set_ps(0.0f, 0.0f, 2.0f / (top - bottom), 0.0f);
	returner.rows[2] = _mm_set_ps(0.0f, 2.0f / (near - far), 0.0f, 0.0f);
	returner.rows[3] = _mm_set_ps(1.0f, (far + near) / (far - near), (bottom + top) / (bottom - top), (left + right) / (left - right));
	return returner;
}

ng::math::Mat4f ng::math::Mat4f::perspective(float fov, float aspectRatio, float near, float far)
{
	Mat4f returner;
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

ng::math::Mat4f ng::math::Mat4f::lookAt(const Vec3f & camera, const Vec3f & object, const Vec3f & up)
{
	Mat4f ret;
	Vec3f f = Vec3f::normalized(object - camera);
	Vec3f s = f.cross(Vec3f::normalized(up));
	Vec3f u = s.cross(Vec3f::normalized(f));
	ret.rows[0] = _mm_set_ps(0.0f, -f.x, u.x, s.x);
	ret.rows[1] = _mm_set_ps(0.0f, -f.y, u.y, s.y);
	ret.rows[2] = _mm_set_ps(0.0f, -f.z, u.z, s.z);
	ret.rows[3] = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	return ret * Mat4f::translation(Vec3f(-camera.x, -camera.y, -camera.z));
}

ng::math::Mat4f ng::math::Mat4f::translation(const Vec3f & translation)
{
	Mat4f returner;
	returner.rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
	returner.rows[1] = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	returner.rows[2] = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	returner.rows[3] = _mm_set_ps(1.0f, translation.z, translation.y, translation.x);
	return returner;
}

ng::math::Mat4f ng::math::Mat4f::rotation(const Vec3f& axis, float angle)
{
#define x axis.x
#define y axis.y
#define z axis.z

	Mat4f returner;
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

ng::math::Mat4f ng::math::Mat4f::rotation(const Quaternion & quat)
{
	Mat4f m1;
	Mat4f m2;

	m1.rows[0] = _mm_set_ps(quat.getX(), -quat.getY(), quat.getZ(), quat.getW());
	m1.rows[1] = _mm_set_ps(quat.getY(), quat.getW(), quat.getW(), -quat.getZ());
	m1.rows[2] = _mm_set_ps(quat.getZ(), -quat.getW(), -quat.getX(), quat.getY());
	m1.rows[3] = _mm_set_ps(quat.getW(), -quat.getZ(), -quat.getY(), -quat.getX());
	
	m2.rows[0] = _mm_set_ps(-quat.getX(), -quat.getY(), quat.getZ(), quat.getW());
	m2.rows[1] = _mm_set_ps(-quat.getY(), quat.getW(), quat.getW(), -quat.getZ());
	m2.rows[2] = _mm_set_ps(-quat.getZ(), -quat.getW(), -quat.getX(), quat.getY());
	m2.rows[3] = _mm_set_ps(quat.getW(), quat.getZ(), quat.getY(), quat.getX());

	return m1 * m2;
}

ng::math::Mat4f ng::math::Mat4f::scale(const Vec3f & scale)
{
	Mat4f returner;
	returner.rows[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, scale.x);
	returner.rows[1] = _mm_set_ps(0.0f, 0.0f, scale.y, 0.0f);
	returner.rows[2] = _mm_set_ps(0.0f, scale.z, 0.0f, 0.0f);
	returner.rows[3] = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	return returner;
}

ng::math::Mat4f ng::math::operator*(Mat4f left, const Mat4f & right)
{
	return left.mul(right);
}

ng::math::Vec3f ng::math::operator*(const Mat4f & left, const Vec3f & right)
{
	return left.mul(right);
}

ng::math::Vec4f ng::math::operator*(const Mat4f & left, const Vec4f & right)
{
	return left.mul(right);
}

ng::math::Mat4f ng::math::operator+(Mat4f left, const Mat4f & right)
{
	return left.add(right);
}

ng::math::Mat4f ng::math::operator-(Mat4f left, const Mat4f & right)
{
	return left.sub(right);
}

std::ostream & ng::math::operator<<(std::ostream & stream, const Mat4f & matrix)
{
	stream << "Mat4f: [\n";
	for (int i = 0; i < 4; ++i) {
		stream << matrix.rows[i].m128_f32[0] << ", " << matrix.rows[i].m128_f32[1] << ", " << matrix.rows[i].m128_f32[2] << ", " << matrix.rows[i].m128_f32[3] << std::endl;
	}
	return stream;
}



#undef _mm_shufd