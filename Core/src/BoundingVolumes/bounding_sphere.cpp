#include "bounding_sphere.h"



ng::bvolumes::BoundingSphere::BoundingSphere()
{

}

ng::bvolumes::BoundingSphere::BoundingSphere(ng::math::Vec3f* center, float radius)
{
	this->center = center;
	this->radius = radius;
}


