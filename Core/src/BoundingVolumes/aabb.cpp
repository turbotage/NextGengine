#include "aabb.h"

ng::bvolumes::AABB::AABB()
{

}

ng::bvolumes::AABB::AABB(ngm::Vec3f * position, ngm::Vec3f max, ngm::Vec3f min)
{
	this->position = position;
	this->max = max;
	this->min = min;
}

ng::bvolumes::AABB::~AABB()
{

}

float ng::bvolumes::AABB::maxX()
{
	return position->x + max.x;
}

void ng::bvolumes::AABB::setMaxX(float maxX)
{
	max.x = maxX - position->x;
}

float ng::bvolumes::AABB::minX()
{
	return position->x + min.x;
}

void ng::bvolumes::AABB::setMinX(float minX)
{
	min.x = minX - position->x;
}

float ng::bvolumes::AABB::maxY()
{
	return position->y + max.y;
}

void ng::bvolumes::AABB::setMaxY(float maxY)
{
	max.y = maxY - position->y;
}

float ng::bvolumes::AABB::minY()
{
	return position->y + min.y;
}

void ng::bvolumes::AABB::setMinY(float minY)
{
	min.y = minY - position->y;
}

float ng::bvolumes::AABB::maxZ()
{
	return position->z + max.z;
}

void ng::bvolumes::AABB::setMaxZ(float maxZ)
{
	max.z = maxZ - position->z;
}

float ng::bvolumes::AABB::minZ()
{
	return position->z + min.z;
}

void ng::bvolumes::AABB::setMinZ(float minZ)
{
	min.z = minZ - position->z;
}

ngm::Vec3f ng::bvolumes::AABB::getPoint0()
{
	return *position + max;
}

ngm::Vec3f ng::bvolumes::AABB::getPoint1()
{
	return *position + ngm::Vec3f(min.x, max.y, max.z);
}

ngm::Vec3f ng::bvolumes::AABB::getPoint2()
{
	return *position + ngm::Vec3f(min.x, max.y, min.z);
}

ngm::Vec3f ng::bvolumes::AABB::getPoint3()
{
	return *position + ngm::Vec3f(max.x, max.y, min.z);
}

ngm::Vec3f ng::bvolumes::AABB::getPoint4()
{
	return *position + ngm::Vec3f(max.x, min.y, max.z);
}

ngm::Vec3f ng::bvolumes::AABB::getPoint5()
{
	return *position + ngm::Vec3f(min.x, min.y, max.z);
}

ngm::Vec3f ng::bvolumes::AABB::getPoint6()
{
	return *position + ngm::Vec3f(min.x, min.y, min.z);
}

ngm::Vec3f ng::bvolumes::AABB::getPoint7()
{
	return *position + ngm::Vec3f(max.x, min.y, min.z);
}



