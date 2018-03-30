#include "plane.h"

ng::math::Plane::Plane()
{
}

ng::math::Plane::Plane(Vec3f planeNormal, Vec3f point)
{
	setPlane(planeNormal, point);
}

void ng::math::Plane::setPlane(Vec3f planeNormal, Vec3f point)
{
	a = planeNormal.x;
	b = planeNormal.y;
	c = planeNormal.z;
	d = ((a * point.x) + (b * point.y) + (c * point.z));
	normOfBasis = sqrt( (a * a) + (b * b) + (c * c) );
}

float ng::math::Plane::distanceToPoint(Vec3f point)
{
	return abs(a*point.x + b * point.y + c * point.z + d) / normOfBasis;
}
