#include "plane.h"

ng::math::Plane::Plane()
{
}

ng::math::Plane::Plane(Vec3 point, Vec3 planeVector, Vec3 planeNormal)
{
	setPlane(point, planeVector, planeNormal);
}

void ng::math::Plane::setPlane(Vec3 point, Vec3 planeVector, Vec3 planeNormal)
{
	a = planeVector.x;
	b = planeVector.y;
	c = planeVector.z;
	d = 0.0f;
	d -= ((a * point.x) + (b * point.y) + (c * point.z));
	normOfBasis = sqrt( (a * a) + (b * b) + (c * c) );
}

float ng::math::Plane::distanceToPoint(Vec3 point)
{
	return abs(a*point.x + b * point.y + c * point.z + d) / normOfBasis;
}
