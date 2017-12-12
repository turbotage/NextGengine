#include "plane.h"

ng::math::Plane::Plane(Vec3 point, Vec3 planeVector, Vec3 planeNormal)
{
	row = _mm_set_ps(0.0f, planeNormal.z, planeNormal.y, planeNormal.z);
	__m128 temp = _mm_mul_ps(_mm_set_ps(0.0f, point.z, point.y, point.x), row);
	__m128 temp1 = _mm_dp_ps(_mm_set_ps(0.0f, planeNormal.z, planeNormal.y, planeNormal.x), temp, 0x000F);
	row = _mm_add_ps(row, temp);
	temp = 
}
