#pragma once

#include "Math/vec3.h"

class AABB
{
private:
	ng::math::Vec3 position;
	float widthX;
	float heightY;
	float depthZ;
	
public:

	AABB();

	~AABB();
};

