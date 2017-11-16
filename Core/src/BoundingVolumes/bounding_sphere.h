#pragma once

#include "../Math/vec3.h"

class BoundingSphere
{
private:
	ng::math::Vec3 center;
	float radius;
public:
	BoundingSphere();
	~BoundingSphere();
};

