#pragma once

#include "src\Math\vec3.h"

class Camera
{
private:

	float nearPlaneDistance;
	float farPlaneDistance;

	bool isInView();

public:
	Camera();
	~Camera();
};

