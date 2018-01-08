#include "quaternion.h"

float ng::math::Quaternion::getX() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(0, 0, 0, 0)));
}

float ng::math::Quaternion::getY() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(1, 1, 1, 1)));
}

float ng::math::Quaternion::getZ() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(2, 2, 2, 2)));
}

float ng::math::Quaternion::getW() const
{
	return _mm_cvtss_f32(_mm_shuffle_ps(row, row, _MM_SHUFFLE(3, 3, 3, 3)));
}

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
	ret.row = _mm_mul_ps(_mm_set_ps(getW(), getW(), getW(), getW()), _mm_set_ps(other.getW(), other.getZ(), other.getY(), other.getX()));
	__m128 temp = _mm_mul_ps(_mm_set_ps(-1.0f * getX(), getZ(), getY(), getX()), _mm_set_ps(other.getX(), other.getW(), other.getW(), other.getW()));
	ret.row = _mm_add_ps(ret.row, temp);
	temp = _mm_mul_ps(_mm_set_ps(-1.0f * getY(), getX(), getZ(), getY()), _mm_set_ps(other.getY(), other.getY(), other.getX(), other.getZ()));
	ret.row = _mm_add_ps(ret.row, temp);
	temp = _mm_mul_ps(_mm_set_ps(getZ(), getY(), getX(), getZ()), _mm_set_ps(other.getZ(), other.getX(), other.getZ(), other.getY()));
	ret.row = _mm_sub_ps(ret.row, temp);
	ret.normalize();
	row = ret.row;
	return *this;
}

ng::math::Quaternion & ng::math::Quaternion::setRotation(const Vec3f & rotationAxis, float angle)
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

ng::math::Quaternion ng::math::Quaternion::getRotation(const Vec3f & rotationAxis, float angle)
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

ng::math::Quaternion & ng::math::Quaternion::setRotation(const Vec4f & rotationAxis, float angle)
{
	float sinAngle = sin(angle*0.5);
	row = _mm_mul_ps(_mm_set_ps(cos(angle*0.5), sinAngle, sinAngle, sinAngle), rotationAxis.row);
	return *this;
}

ng::math::Quaternion ng::math::Quaternion::getRotation(const Vec4f & rotationAxis, float angle)
{
	Quaternion quat;
	float sinAngle = sin(angle*0.5);
	quat.row = _mm_mul_ps(_mm_set_ps(cos(angle*0.5), sinAngle, sinAngle, sinAngle), rotationAxis.row);
	return quat;
}

ng::math::Vec3f ng::math::Quaternion::eulerAngles()
{
	return Vec3f(
		atan2(2.0f * (getX() * getY() + getZ() * getW()), 1.0f - 2.0f * (getY() * getY() + getZ() * getZ()))
		, asin(2.0f * (getX() * getW() + getY() * getZ()))
		, atan2(2.0f * (getX() * getW() + getY() * getZ()), 1.0f - 2.0f * (getZ() * getZ() + getW() * getW()))
		);
}

ng::math::Vec3f ng::math::Quaternion::getEulerAngles(const Quaternion & quat)
{
	return Vec3f(
		atan2(2.0f * (quat.getX() * quat.getY() + quat.getZ() * quat.getW()), 1.0f - 2.0f * (quat.getY() * quat.getY() + quat.getZ() * quat.getZ()))
		, asin(2.0f * (quat.getX() * quat.getW() + quat.getY() * quat.getZ()))
		, atan2(2.0f * (quat.getX() * quat.getW() + quat.getY() * quat.getZ()), 1.0f - 2.0f * (quat.getZ() * quat.getZ() + quat.getW() * quat.getW()))
	);
}

ng::math::Vec3f ng::math::Quaternion::rotate(const Vec3f & vec)
{
	__m128 temp1 = _mm_mul_ps(_mm_set_ps(getX(), getW(), getW(), getW()), _mm_set_ps(vec.x, vec.z, vec.y, vec.x));
	__m128 temp2 = _mm_mul_ps(_mm_set_ps(getY(), getX(), getZ(), getY()), _mm_set_ps(vec.y, vec.y, vec.x, vec.z));
	__m128 temp3 = _mm_mul_ps(_mm_set_ps(-getZ(), getY(), getX(), getZ()), _mm_set_ps(vec.z, vec.x, vec.z, vec.y));
	temp1 = _mm_add_ps(temp1, temp2);
	temp1 = _mm_sub_ps(temp1, temp3);
	ALIGN(16) float temps[4];
	_mm_store_ps(temps, temp1);

	temp1 = _mm_set_ps(0.0f, temps[0], temps[0], temps[0]);
	temp2 = _mm_set_ps(0.0f, getZ(), getY(), getX());
	temp1 = _mm_mul_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[1], temps[2], temps[3]);
	temp3 = _mm_set_ps(0.0f, getW(), getW(), getW());
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_add_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[3], temps[1], temps[2]);
	temp3 = _mm_set_ps(0.0f, getY(), getX(), getZ());
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_sub_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[2], temps[3], temps[1]);
	temp3 = _mm_set_ps(0.0f, getX(), getZ(), getY());
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_add_ps(temp1, temp2);
	_mm_store_ps(temps, temp1);
	return Vec3f(temps[3], temps[2], temps[1]);
}

ng::math::Vec3f ng::math::Quaternion::getRotation(const Quaternion & quat, const Vec3f & vec)
{
	__m128 temp1 = _mm_mul_ps(_mm_set_ps(quat.getX(), quat.getW(), quat.getW(), quat.getW()), _mm_set_ps(vec.x, vec.z, vec.y, vec.x));
	__m128 temp2 = _mm_mul_ps(_mm_set_ps(quat.getY(), quat.getX(), quat.getZ(), quat.getY()), _mm_set_ps(vec.y, vec.y, vec.x, vec.z));
	__m128 temp3 = _mm_mul_ps(_mm_set_ps(-quat.getZ(), quat.getY(), quat.getX(), quat.getZ()), _mm_set_ps(vec.z, vec.x, vec.z, vec.y));
	temp1 = _mm_add_ps(temp1, temp2);
	temp1 = _mm_sub_ps(temp1, temp3);
	ALIGN(16) float temps[4];
	_mm_store_ps(temps, temp1);

	temp1 = _mm_set_ps(0.0f, temps[0], temps[0], temps[0]);
	temp2 = _mm_set_ps(0.0f, quat.getZ(), quat.getY(), quat.getX());
	temp1 = _mm_mul_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[1], temps[2], temps[3]);
	temp3 = _mm_set_ps(0.0f, quat.getW(), quat.getW(), quat.getW());
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_add_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[3], temps[1], temps[2]);
	temp3 = _mm_set_ps(0.0f, quat.getY(), quat.getX(), quat.getZ());
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_sub_ps(temp1, temp2);

	temp2 = _mm_set_ps(0.0f, temps[2], temps[3], temps[1]);
	temp3 = _mm_set_ps(0.0f, quat.getX(), quat.getZ(), quat.getY());
	temp2 = _mm_mul_ps(temp2, temp3);

	temp1 = _mm_add_ps(temp1, temp2);
	_mm_store_ps(temps, temp1);
	return Vec3f(temps[3], temps[2], temps[1]);
}

void ng::math::Quaternion::rotate4(const Quaternion & quat, Vec3f & v1, Vec3f & v2, Vec3f & v3, Vec3f & v4)
{
	__m128 tempX;
	{
		tempX = _mm_mul_ps(_mm_set_ps1(quat.getW()), _mm_set_ps(v4.x, v3.x, v2.x, v1.x));
		tempX = _mm_add_ps(tempX, _mm_mul_ps(_mm_set_ps1(quat.getY()), _mm_set_ps(v4.z, v3.z, v2.z, v1.z)));
		tempX = _mm_sub_ps(tempX, _mm_mul_ps(_mm_set_ps1(quat.getZ()), _mm_set_ps(v4.y, v3.y, v2.y, v1.y)));
	}
	__m128 tempY;
	{
		tempY = _mm_mul_ps(_mm_set_ps1(quat.getW()), _mm_set_ps(v4.y, v3.y, v2.y, v1.y));
		tempY = _mm_add_ps(tempY, _mm_mul_ps(_mm_set_ps1(quat.getZ()), _mm_set_ps(v4.x, v3.x, v2.x, v1.x)));
		tempY = _mm_sub_ps(tempY, _mm_mul_ps(_mm_set_ps1(quat.getX()), _mm_set_ps(v4.z, v3.z, v2.z, v1.z)));
	}
	__m128 tempZ;
	{
		tempZ = _mm_mul_ps(_mm_set_ps1(quat.getW()), _mm_set_ps(v4.z, v3.z, v2.z, v1.z));
		tempZ = _mm_add_ps(tempZ, _mm_mul_ps(_mm_set_ps1(quat.getX()), _mm_set_ps(v4.y, v3.y, v2.y, v1.y)));
		tempZ = _mm_sub_ps(tempZ, _mm_mul_ps(_mm_set_ps1(quat.getY()), _mm_set_ps(v4.x, v3.x, v2.x, v1.x)));
	}
	__m128 tempW;
	{
		tempW = _mm_mul_ps(_mm_set_ps1(quat.getX()), _mm_set_ps(v4.x, v3.x, v2.x, v1.x));
		tempW = _mm_add_ps(tempW, _mm_mul_ps(_mm_set_ps1(quat.getY()), _mm_set_ps(v4.y, v3.y, v2.y, v1.y)));
		tempW = _mm_add_ps(tempW, _mm_mul_ps(_mm_set_ps1(quat.getZ()), _mm_set_ps(v4.z, v3.z, v2.z, v1.z)));
	}
	__m128 X;
	{
		X = _mm_add_ps(_mm_mul_ps(tempW, _mm_set_ps1(quat.getX())), _mm_mul_ps(tempX, _mm_set_ps1(quat.getW())));
		X = _mm_sub_ps(X, _mm_mul_ps(tempY, _mm_set_ps1(quat.getZ())));
		X = _mm_add_ps(X, _mm_mul_ps(tempZ, _mm_set_ps1(quat.getY())));
	}
	__m128 Y;
	{
		Y = _mm_add_ps(_mm_mul_ps(tempW, _mm_set_ps1(quat.getY())), _mm_mul_ps(tempY, _mm_set_ps1(quat.getW())));
		Y = _mm_sub_ps(Y, _mm_mul_ps(tempZ, _mm_set_ps1(quat.getX())));
		Y = _mm_add_ps(Y, _mm_mul_ps(tempX, _mm_set_ps1(quat.getZ())));
	}
	__m128 Z;
	{
		Z = _mm_add_ps(_mm_mul_ps(tempW, _mm_set_ps1(quat.getZ())), _mm_mul_ps(tempZ, _mm_set_ps1(quat.getW())));
		Z = _mm_sub_ps(Z, _mm_mul_ps(tempX, _mm_set_ps1(quat.getY())));
		Z = _mm_add_ps(Z, _mm_mul_ps(tempY, _mm_set_ps1(quat.getX())));
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

float ng::math::Quaternion::norm()
{
	Quaternion q;
	q.row = _mm_dp_ps(row, row, 0xFFFF);
	return q.getX();
}

float ng::math::Quaternion::norm(const Quaternion & quat)
{
	Quaternion q;
	q.row = _mm_dp_ps(quat.row, quat.row, 0xFFFF);
	return q.getX();
}

float ng::math::Quaternion::length()
{
	return sqrt(norm());
}

float ng::math::Quaternion::length(const Quaternion & quat)
{
	return sqrt(norm(quat));
}

ng::math::Quaternion & ng::math::Quaternion::normalize()
{
	Quaternion q;
	q.row = _mm_dp_ps(row, row, 0xFFFF);
	q.row = _mm_rsqrt_ps(q.row);
	row = _mm_mul_ps(q.row, row);
	return *this;
}

ng::math::Quaternion ng::math::Quaternion::normalized(const Quaternion & quat)
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
