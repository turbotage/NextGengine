#include "quaternion.h"

float ng::math::Quaternion::getX() const
{
	return _mcvtss_f32(_mshuffle_ps(row, row, _MSHUFFLE(0, 0, 0, 0)));
}

float ng::math::Quaternion::getY() const
{
	return _mcvtss_f32(_mshuffle_ps(row, row, _MSHUFFLE(1, 1, 1, 1)));
}

float ng::math::Quaternion::getZ() const
{
	return _mcvtss_f32(_mshuffle_ps(row, row, _MSHUFFLE(2, 2, 2, 2)));
}

float ng::math::Quaternion::getW() const
{
	return _mcvtss_f32(_mshuffle_ps(row, row, _MSHUFFLE(3, 3, 3, 3)));
}

ng::math::Quaternion::Quaternion()
{
	_mset_ps(0.0f, 0.0f, 0.0f, 0.0f);
}

ng::math::Quaternion::Quaternion(float scalar)
{
	_mset_ps(scalar, scalar, scalar, scalar);
}

ng::math::Quaternion::Quaternion(float x, float y, float z, float w)
{
	_mset_ps(w, z, y, x);
}

ng::math::Quaternion::Quaternion(const Quaternion & Quaternion)
{
	row = Quaternion.row;
}

ng::math::Quaternion & ng::math::Quaternion::add(const Quaternion & other)
{
	row = _madd_ps(row, other.row);
	return *this;
}

ng::math::Quaternion & ng::math::Quaternion::operator+=(const Quaternion & other)
{
	return add(other);
}

ng::math::Quaternion & ng::math::Quaternion::sub(const Quaternion & other)
{
	row = _msub_ps(row, other.row);
	return *this;
}

ng::math::Quaternion & ng::math::Quaternion::operator-=(const Quaternion & other)
{
	return sub(other);
}

ng::math::Quaternion & ng::math::Quaternion::mul(const Quaternion & other)
{
	Quaternion ret;
	ret.row = _mmul_ps(_mset_ps(getW(), getW(), getW(), getW()), _mset_ps(other.getW(), other.getZ(), other.getY(), other.getX()));
	__m128 temp = _mmul_ps(_mset_ps(-1.0f * getX(), getZ(), getY(), getX()), _mset_ps(other.getX(), other.getW(), other.getW(), other.getW()));
	ret.row = _madd_ps(ret.row, temp);
	temp = _mmul_ps(_mset_ps(-1.0f * getY(), getX(), getZ(), getY()), _mset_ps(other.getY(), other.getY(), other.getX(), other.getZ()));
	ret.row = _madd_ps(ret.row, temp);
	temp = _mmul_ps(_mset_ps(getZ(), getY(), getX(), getZ()), _mset_ps(other.getZ(), other.getX(), other.getZ(), other.getY()));
	ret.row = _msub_ps(ret.row, temp);
	ret.normalize();
	row = ret.row;
	return *this;
}

ng::math::Quaternion & ng::math::Quaternion::operator*=(const Quaternion & other)
{
	return mul(other);
}

ng::math::Quaternion & ng::math::Quaternion::setRotation(const Vec3f & rotationAxis, float angle)
{
	float sinAngle = sin(angle*0.5);
	row = _mset_ps(
		cos(angle*0.5),
		rotationAxis.z * sinAngle,
		rotationAxis.y * sinAngle,
		rotationAxis.x * sinAngle
	);
	return *this;
}

ng::math::Quaternion ng::math::Quaternion::getQuaternion(const Vec3f & rotationAxis, float angle)
{
	Quaternion quat;
	float sinAngle = sin(angle*0.5);
	quat.row = _mset_ps(
		cos(angle*0.5),
		rotationAxis.z * sinAngle,
		rotationAxis.y * sinAngle,
		rotationAxis.x * sinAngle
	);
	return quat;
}

ng::math::Quaternion & ng::math::Quaternion::setRotation(const Vec4f & rotationAxis, float angle)
{
	float sinAngle = sin(angle*0.5);
	row = _mmul_ps(_mset_ps(cos(angle*0.5), sinAngle, sinAngle, sinAngle), rotationAxis.row);
	return *this;
}

ng::math::Quaternion ng::math::Quaternion::getQuaternion(const Vec4f & rotationAxis, float angle)
{
	Quaternion quat;
	float sinAngle = sin(angle*0.5);
	quat.row = _mmul_ps(_mset_ps(cos(angle*0.5), sinAngle, sinAngle, sinAngle), rotationAxis.row);
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

void ng::math::Quaternion::rotate(Vec3f & vec)
{
	__m128 temp1 = _mmul_ps(_mset_ps(getX(), getW(), getW(), getW()), _mset_ps(vec.x, vec.z, vec.y, vec.x));
	__m128 temp2 = _mmul_ps(_mset_ps(getY(), getX(), getZ(), getY()), _mset_ps(vec.y, vec.y, vec.x, vec.z));
	__m128 temp3 = _mmul_ps(_mset_ps(-getZ(), getY(), getX(), getZ()), _mset_ps(vec.z, vec.x, vec.z, vec.y));
	temp1 = _madd_ps(temp1, temp2);
	temp1 = _msub_ps(temp1, temp3);
	ALIGN(16) float temps[4];
	_mstore_ps(temps, temp1);

	temp1 = _mset_ps(0.0f, temps[0], temps[0], temps[0]);
	temp2 = _mset_ps(0.0f, getZ(), getY(), getX());
	temp1 = _mmul_ps(temp1, temp2);

	temp2 = _mset_ps(0.0f, temps[1], temps[2], temps[3]);
	temp3 = _mset_ps(0.0f, getW(), getW(), getW());
	temp2 = _mmul_ps(temp2, temp3);

	temp1 = _madd_ps(temp1, temp2);

	temp2 = _mset_ps(0.0f, temps[3], temps[1], temps[2]);
	temp3 = _mset_ps(0.0f, getY(), getX(), getZ());
	temp2 = _mmul_ps(temp2, temp3);

	temp1 = _msub_ps(temp1, temp2);

	temp2 = _mset_ps(0.0f, temps[2], temps[3], temps[1]);
	temp3 = _mset_ps(0.0f, getX(), getZ(), getY());
	temp2 = _mmul_ps(temp2, temp3);

	temp1 = _madd_ps(temp1, temp2);
	_mstore_ps(temps, temp1);
	vec.x = temps[3];
	vec.y = temps[2];
	vec.z = temps[1];
}

ng::math::Vec3f ng::math::Quaternion::getRotation(const Quaternion & quat, const Vec3f & vec)
{
	__m128 temp1 = _mmul_ps(_mset_ps(quat.getX(), quat.getW(), quat.getW(), quat.getW()), _mset_ps(vec.x, vec.z, vec.y, vec.x));
	__m128 temp2 = _mmul_ps(_mset_ps(quat.getY(), quat.getX(), quat.getZ(), quat.getY()), _mset_ps(vec.y, vec.y, vec.x, vec.z));
	__m128 temp3 = _mmul_ps(_mset_ps(-quat.getZ(), quat.getY(), quat.getX(), quat.getZ()), _mset_ps(vec.z, vec.x, vec.z, vec.y));
	temp1 = _madd_ps(temp1, temp2);
	temp1 = _msub_ps(temp1, temp3);
	ALIGN(16) float temps[4];
	_mstore_ps(temps, temp1);

	temp1 = _mset_ps(0.0f, temps[0], temps[0], temps[0]);
	temp2 = _mset_ps(0.0f, quat.getZ(), quat.getY(), quat.getX());
	temp1 = _mmul_ps(temp1, temp2);

	temp2 = _mset_ps(0.0f, temps[1], temps[2], temps[3]);
	temp3 = _mset_ps(0.0f, quat.getW(), quat.getW(), quat.getW());
	temp2 = _mmul_ps(temp2, temp3);

	temp1 = _madd_ps(temp1, temp2);

	temp2 = _mset_ps(0.0f, temps[3], temps[1], temps[2]);
	temp3 = _mset_ps(0.0f, quat.getY(), quat.getX(), quat.getZ());
	temp2 = _mmul_ps(temp2, temp3);

	temp1 = _msub_ps(temp1, temp2);

	temp2 = _mset_ps(0.0f, temps[2], temps[3], temps[1]);
	temp3 = _mset_ps(0.0f, quat.getX(), quat.getZ(), quat.getY());
	temp2 = _mmul_ps(temp2, temp3);

	temp1 = _madd_ps(temp1, temp2);
	_mstore_ps(temps, temp1);
	return Vec3f(temps[3], temps[2], temps[1]);
}

void ng::math::Quaternion::rotate4(Vec3f & v1, Vec3f & v2, Vec3f & v3, Vec3f & v4)
{
	__m128 tempX;
	{
		tempX = _mmul_ps(_mset_ps1(getW()), _mset_ps(v4.x, v3.x, v2.x, v1.x));
		tempX = _madd_ps(tempX, _mmul_ps(_mset_ps1(getY()), _mset_ps(v4.z, v3.z, v2.z, v1.z)));
		tempX = _msub_ps(tempX, _mmul_ps(_mset_ps1(getZ()), _mset_ps(v4.y, v3.y, v2.y, v1.y)));
	}
	__m128 tempY;
	{
		tempY = _mmul_ps(_mset_ps1(getW()), _mset_ps(v4.y, v3.y, v2.y, v1.y));
		tempY = _madd_ps(tempY, _mmul_ps(_mset_ps1(getZ()), _mset_ps(v4.x, v3.x, v2.x, v1.x)));
		tempY = _msub_ps(tempY, _mmul_ps(_mset_ps1(getX()), _mset_ps(v4.z, v3.z, v2.z, v1.z)));
	}
	__m128 tempZ;
	{
		tempZ = _mmul_ps(_mset_ps1(getW()), _mset_ps(v4.z, v3.z, v2.z, v1.z));
		tempZ = _madd_ps(tempZ, _mmul_ps(_mset_ps1(getX()), _mset_ps(v4.y, v3.y, v2.y, v1.y)));
		tempZ = _msub_ps(tempZ, _mmul_ps(_mset_ps1(getY()), _mset_ps(v4.x, v3.x, v2.x, v1.x)));
	}
	__m128 tempW;
	{
		tempW = _mmul_ps(_mset_ps1(getX()), _mset_ps(v4.x, v3.x, v2.x, v1.x));
		tempW = _madd_ps(tempW, _mmul_ps(_mset_ps1(getY()), _mset_ps(v4.y, v3.y, v2.y, v1.y)));
		tempW = _madd_ps(tempW, _mmul_ps(_mset_ps1(getZ()), _mset_ps(v4.z, v3.z, v2.z, v1.z)));
	}
	__m128 X;
	{
		X = _madd_ps(_mmul_ps(tempW, _mset_ps1(getX())), _mmul_ps(tempX, _mset_ps1(getW())));
		X = _msub_ps(X, _mmul_ps(tempY, _mset_ps1(getZ())));
		X = _madd_ps(X, _mmul_ps(tempZ, _mset_ps1(getY())));
	}
	__m128 Y;
	{
		Y = _madd_ps(_mmul_ps(tempW, _mset_ps1(getY())), _mmul_ps(tempY, _mset_ps1(getW())));
		Y = _msub_ps(Y, _mmul_ps(tempZ, _mset_ps1(getX())));
		Y = _madd_ps(Y, _mmul_ps(tempX, _mset_ps1(getZ())));
	}
	__m128 Z;
	{
		Z = _madd_ps(_mmul_ps(tempW, _mset_ps1(getZ())), _mmul_ps(tempZ, _mset_ps1(getW())));
		Z = _msub_ps(Z, _mmul_ps(tempX, _mset_ps1(getY())));
		Z = _madd_ps(Z, _mmul_ps(tempY, _mset_ps1(getX())));
	}
	float xFloats[4];
	float yFloats[4];
	float zFloats[4];
	_mstore_ps(xFloats, X);
	_mstore_ps(yFloats, Y);
	_mstore_ps(zFloats, Z);
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
	q.row = _mdp_ps(row, row, 0xFFFF);
	return q.getX();
}

float ng::math::Quaternion::norm(const Quaternion & quat)
{
	Quaternion q;
	q.row = _mdp_ps(quat.row, quat.row, 0xFFFF);
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
	q.row = _mdp_ps(row, row, 0xFFFF);
	q.row = _mrsqrt_ps(q.row);
	row = _mmul_ps(q.row, row);
	return *this;
}

ng::math::Quaternion ng::math::Quaternion::normalized(const Quaternion & quat)
{
	Quaternion q;
	q.row = _mdp_ps(quat.row, quat.row, 0xFFFF);
	q.row = _mrsqrt_ps(q.row);
	q.row = _mmul_ps(q.row, quat.row);
	return q;
}

ng::math::Quaternion::~Quaternion()
{
	
}

