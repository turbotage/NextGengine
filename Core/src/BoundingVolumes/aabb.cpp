#include "aabb.h"

float ng::bvolumes::AABB::maxX()
{
	return position->x + positiveWidth;
}

void ng::bvolumes::AABB::setMaxX(float maxX)
{
	positiveWidth = maxX - position->x;
}

float ng::bvolumes::AABB::minX()
{
	return position->x + negativeWidth;
}

void ng::bvolumes::AABB::setMinX(float minX)
{
	negativeWidth = minX - position->x;
}

float ng::bvolumes::AABB::maxY()
{
	return position->y + positiveHeight;
}

void ng::bvolumes::AABB::setMaxY(float maxY)
{
	positiveHeight = maxY - position->y;
}

float ng::bvolumes::AABB::minY()
{
	return position->y + negativeHeight;
}

void ng::bvolumes::AABB::setMinY(float minY)
{
	negativeHeight = minY - position->y;
}

float ng::bvolumes::AABB::maxZ()
{
	return position->z + positiveDepth;
}

void ng::bvolumes::AABB::setMaxZ(float maxZ)
{
	positiveDepth = maxZ - position->z;
}

float ng::bvolumes::AABB::minZ()
{
	return position->z + negativeDepth;
}

void ng::bvolumes::AABB::setMinZ(float minZ)
{
	negativeDepth = minZ - position->z;
}

