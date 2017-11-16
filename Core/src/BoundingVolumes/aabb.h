#pragma once

#include "../Math/vec3.h"

class AABB
{
private:
	//aabb 
	//	1		0
	//	|	2	|	3
	//	|	|	|	|
	//	5	|	4	|
	//		6		7
	ng::math::Vec3 center;
	float width; // x
	float height; // y
	float depth; //z
	
public:

	AABB();

	~AABB();
};

