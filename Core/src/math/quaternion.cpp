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

ng::math::Quaternion ng::math::Quaternion::rotation(const Vec3 & rotationAxis, float angle)
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

ng::math::Quaternion ng::math::Quaternion::rotation(const Vec4 & rotationAxis, float angle)
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
	
}

ng::math::Quaternion::~Quaternion()
{
	
}
