#pragma once

#include "../Math/vec3.h"

class GeneralBV
{
private:
	//the position of an generalBV is not defined in this class
	
	//aabb 
	//	1		0
	//	|	2	|	3
	//	|	|	|	|
	//	5	|	4	|
	//		6		7
	float width; //x
	float height; //y
	float depth; //z

	//bounding sphere
	float radius;
public:
	GeneralBV();
	~GeneralBV();
};

