#include "bounding_sphere.h"



ng::bvolumes::BoundingSphere::BoundingSphere()
	: m_Center(ng::math::Vec3f(0.0f, 0.0f, 0.0f)), m_Radius(0.0f)
{

}

ng::bvolumes::BoundingSphere::BoundingSphere(ng::math::Vec3f pos, float radius)
	: m_Center(pos), m_Radius(radius)
{

}


