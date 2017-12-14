#pragma once

#include "Math\plane.h"
#include "Math\quaternion.h"

class Camera
{
private:



	bool isInView();

public:

	float fieldOfView;

	float farPlaneDistance;
	float nearPlaneDistance;
	float halfWidth;
	float halfHeight;

	float angleChangeSinceLastUpdate;
	ng::math::Quaternion m_Rotation;

	ng::math::Plane rightPlane;
	ng::math::Plane leftPlane;
	ng::math::Plane topPlane;
	ng::math::Plane bottomPlane;

	ng::math::Plane nearPlane;
	ng::math::Plane farPlane;
	ng::math::Plane cullPlane1;
	ng::math::Plane	cullPlane2;

	ng::math::Vec3 m_Forward;
	ng::math::Vec3 m_Up;
	ng::math::Vec3 m_Right;
	ng::math::Vec3 m_Left;

	ng::math::Vec3* position;

	Camera(float fov, float viewDistance, float htwRatio);
	~Camera();
};

