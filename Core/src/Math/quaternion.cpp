#include "quaternion.h"

ng::math::Quaternion::Quaternion()
{
	_mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
}

ng::math::Quaternion::Quaternion(float scalar)
{
	_mm_set_ps(scalar, scalar, scalar, scalar);
}

ng::math::Quaternion::Quaternion(float x, float y, float z, float w)
{
	_mm_set_ps(x, y, z, w);
}

ng::math::Quaternion::Quaternion(const Quaternion & Quaternion)
{
	row = Quaternion.row;
}

ng::math::Quaternion & ng::math::Quaternion::add(const Quaternion & other)
{
	row = _mm_add_ps(row, other.row);
	return *this;
}

ng::math::Quaternion & ng::math::Quaternion::sub(const Quaternion & other)
{
	row = _mm_sub_ps(row, other.row);
	return *this;
}

ng::math::Quaternion & ng::math::Quaternion::mul(const Quaternion & other)
{
	Quaternion ret;
	ret.row = _mm_mul_ps(_mm_set_ps(w, w, w, w), _mm_set_ps(other.w, other.z, other.y, other.x));
	__m128 temp = _mm_mul_ps(_mm_set_ps(-1.0f * x, z, y, x), _mm_set_ps(other.x, other.w, other.w, other.w));
	ret.row = _mm_add_ps(ret.row, temp);
	temp = _mm_mul_ps(_mm_set_ps(-1.0f * y, x, z, y), _mm_set_ps(other.y, other.y, other.x, other.z));
	ret.row = _mm_add_ps(ret.row, temp);
	temp = _mm_mul_ps(_mm_set_ps(z, y, x, z), _mm_set_ps(other.z, other.x, other.z, other.y));
	ret.row = _mm_sub_ps(ret.row, temp);
	ret.Normalize();
	row = ret.row;
	return *this;
}

ng::math::Quaternion & ng::math::Quaternion::setRotation(const Vec3 & rotationAxis, float angle)
{
	float sinAngle = sin(angle*0.5);
	row = _mm_set_ps(
		rotationAxis.x * sinAngle,
		rotationAxis.y * sinAngle,
		rotationAxis.z * sinAngle,
		cos(angle*0.5)
	);
	return *this;
}

ng::math::Quaternion ng::math::Quaternion::getRotation(const Vec3 & rotationAxis, float angle)
{
	Quaternion quat;
	float sinAngle = sin(angle*0.5);
	quat.row = _mm_set_ps(
		rotationAxis.x * sinAngle,
		rotationAxis.y * sinAngle,
		rotationAxis.z * sinAngle,
		cos(angle*0.5)
	);
	return quat;
}

ng::math::Quaternion & ng::math::Quaternion::setRotation(const Vec4 & rotationAxis, float angle)
{
	float sinAngle = sin(angle*0.5);
	row = _mm_mul_ps(_mm_set_ps(cos(angle*0.5), sinAngle, sinAngle, sinAngle), rotationAxis.row);
	return *this;
}

ng::math::Quaternion ng::math::Quaternion::getRotation(const Vec4 & rotationAxis, float angle)
{
	Quaternion quat;
	float sinAngle = sin(angle*0.5);
	quat.row = _mm_mul_ps(_mm_set_ps(cos(angle*0.5), sinAngle, sinAngle, sinAngle), rotationAxis.row);
	return quat;
}

ng::math::Vec3 ng::math::Quaternion::Rotate(const Quaternion & quat, const Vec3 & vec)
{
	__m128 temp1 = _mm_mul_ps(_mm_set_ps(quat.x, quat.w, quat.w, quat.w), _mm_set_ps(vec.x, vec.z, vec.y, vec.x));
	__m128 temp2 = _mm_mul_ps(_mm_set_ps(quat.y, quat.x, quat.z, quat.y), _mm_set_ps(vec.y, vec.y, vec.x, vec.z));
	__m128 temp3 = _mm_mul_ps(_mm_set_ps(-quat.z, quat.y, quat.x, quat.z), _mm_set_ps(vec.z, vec.x, vec.z, vec.y));
	temp1 = _mm_add_ps(temp1, temp2);
	temp1 = _mm_sub_ps(temp1, temp3);
	ALIGN(16) float temps[4];
	_mm_store_ps(temps, temp1);

	temp1 = _mm_set_ps(0.0f, temps[0], temps[0], temps[0]);
	temp2 = _mm_set_ps(0.0f, quat.z, quat.y, quat.x);
	temp1 = _mm_mul_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[1], temps[2], temps[3]);
	temp3 = _mm_set_ps(0.0f, quat.w, quat.w, quat.w);
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_add_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[3], temps[1], temps[2]);
	temp3 = _mm_set_ps(0.0f, quat.y, quat.x, quat.z);
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_sub_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[2], temps[3], temps[1]);
	temp3 = _mm_set_ps(0.0f, quat.x, quat.z, quat.y);
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_add_ps(temp1, temp2);
	_mm_store_ps(temps, temp1);
	return Vec3(temps[3], temps[2], temps[1]);
}

void ng::math::Quaternion::Rotate4(const Quaternion & quat, Vec3 & v1, Vec3 & v2, Vec3 & v3, Vec3 & v4)
{
	__m128 tempX;
	{
		tempX = _mm_mul_ps(_mm_set_ps1(quat.w), _mm_set_ps(v4.x, v3.x, v2.x, v1.x));
		tempX = _mm_add_ps(tempX, _mm_mul_ps(_mm_set_ps1(quat.y), _mm_set_ps(v4.z, v3.z, v2.z, v1.z)));
		tempX = _mm_sub_ps(tempX, _mm_mul_ps(_mm_set_ps1(quat.z), _mm_set_ps(v4.y, v3.y, v2.y, v1.y)));
	}
	__m128 tempY;
	{
		tempY = _mm_mul_ps(_mm_set_ps1(quat.w), _mm_set_ps(v4.y, v3.y, v2.y, v1.y));
		tempY = _mm_add_ps(tempY, _mm_mul_ps(_mm_set_ps1(quat.z), _mm_set_ps(v4.x, v3.x, v2.x, v1.x)));
		tempY = _mm_sub_ps(tempY, _mm_mul_ps(_mm_set_ps1(quat.x), _mm_set_ps(v4.z, v3.z, v2.z, v1.z)));
	}
	__m128 tempZ;
	{
		tempZ = _mm_mul_ps(_mm_set_ps1(quat.w), _mm_set_ps(v4.z, v3.z, v2.z, v1.z));
		tempZ = _mm_add_ps(tempZ, _mm_mul_ps(_mm_set_ps1(quat.x), _mm_set_ps(v4.y, v3.y, v2.y, v1.y)));
		tempZ = _mm_sub_ps(tempZ, _mm_mul_ps(_mm_set_ps1(quat.y), _mm_set_ps(v4.x, v3.x, v2.x, v1.x)));
	}
	__m128 tempW;
	{
		tempW = _mm_mul_ps(_mm_set_ps1(quat.x), _mm_set_ps(v4.x, v3.x, v2.x, v1.x));
		tempW = _mm_add_ps(tempW, _mm_mul_ps(_mm_set_ps1(quat.y), _mm_set_ps(v4.y, v3.y, v2.y, v1.y)));
		tempW = _mm_add_ps(tempW, _mm_mul_ps(_mm_set_ps1(quat.z), _mm_set_ps(v4.z, v3.z, v2.z, v1.z)));
	}
	__m128 X;
	{
		X = _mm_add_ps(_mm_mul_ps(tempW, _mm_set_ps1(quat.x)), _mm_mul_ps(tempX, _mm_set_ps1(quat.w)));
		X = _mm_sub_ps(X, _mm_mul_ps(tempY, _mm_set_ps1(quat.z)));
		X = _mm_add_ps(X, _mm_mul_ps(tempZ, _mm_set_ps1(quat.y)));
	}
	__m128 Y;
	{
		Y = _mm_add_ps(_mm_mul_ps(tempW, _mm_set_ps1(quat.y)), _mm_mul_ps(tempY, _mm_set_ps1(quat.w)));
		Y = _mm_sub_ps(Y, _mm_mul_ps(tempZ, _mm_set_ps1(quat.x)));
		Y = _mm_add_ps(Y, _mm_mul_ps(tempX, _mm_set_ps1(quat.z)));
	}
	__m128 Z;
	{
		Z = _mm_add_ps(_mm_mul_ps(tempW, _mm_set_ps1(quat.z)), _mm_mul_ps(tempZ, _mm_set_ps1(quat.w)));
		Z = _mm_sub_ps(Z, _mm_mul_ps(tempX, _mm_set_ps1(quat.y)));
		Z = _mm_add_ps(Z, _mm_mul_ps(tempY, _mm_set_ps1(quat.x)));
	}
	float xFloats[4];
	float yFloats[4];
	float zFloats[4];
	_mm_store_ps(xFloats, X);
	_mm_store_ps(yFloats, Y);
	_mm_store_ps(zFloats, Z);
	v1.x = xFloats[3];
	v1.y = yFloats[3];
	v1.z = zFloats[3];

	v2.x = xFloats[2];
	v2.y = yFloats[2];
	v2.z = zFloats[2];

	v3.x = xFloats[1];
	v3.y = yFloats[1];
	v3.z = zFloats[1];

	v4.x = xFloats[0];
	v4.y = yFloats[0];
	v4.z = zFloats[0];
}

float ng::math::Quaternion::Norm()
{
	Quaternion q;
	q.row = _mm_dp_ps(row, row, 0xFFFF);
	return q.x;
}

float ng::math::Quaternion::Norm(const Quaternion & quat)
{
	Quaternion q;
	q.row = _mm_dp_ps(quat.row, quat.row, 0xFFFF);
	return q.x;
}

float ng::math::Quaternion::Length()
{
	return sqrt(Norm());
}

float ng::math::Quaternion::Length(const Quaternion & quat)
{
	return sqrt(Norm(quat));
}

ng::math::Quaternion & ng::math::Quaternion::Normalize()
{
	Quaternion q;
	q.row = _mm_dp_ps(row, row, 0xFFFF);
	q.row = _mm_rsqrt_ps(q.row);
	row = _mm_mul_ps(q.row, row);
	return *this;
}

ng::math::Quaternion ng::math::Quaternion::Normalized(const Quaternion & quat)
{
	Quaternion q;
	q.row = _mm_dp_ps(quat.row, quat.row, 0xFFFF);
	q.row = _mm_rsqrt_ps(q.row);
	q.row = _mm_mul_ps(q.row, quat.row);
	return q;
}

ng::math::Quaternion::~Quaternion()
{
	
}
