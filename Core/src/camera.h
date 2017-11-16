#pragma once

#include "Math\vec3.h"

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

