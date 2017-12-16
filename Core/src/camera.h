#pragma once

#include "Math\plane.h"
#include "Math\quaternion.h"

class Camera
{
private:



	bool isInView();

public:


	const float fieldOfView;
	
	const float farPlaneDistance;
	float nearPlaneDistance;
	float halfWidth;
	float halfHeight;

	float angleChangeSinceLastUpdate;
	bool lastFrame;
	ng::math::Quaternion m_Rotation;

	ng::math::Plane rightPlane;
	ng::math::Plane leftPlane;
	ng::math::Plane topPlane;
	ng::math::Plane bottomPlane;

	ng::math::Plane nearPlane;
	ng::math::Plane farPlane;
	ng::math::Plane cullPlane1;
	ng::math::Plane	cullPlane2;

	ng::math::Vec3 forward;
	ng::math::Vec3 up;
	ng::math::Vec3 right;

	ng::math::Vec3 rightPlaneNormal;
	ng::math::Vec3 leftPlaneNormal;
	ng::math::Vec3 topPlaneNormal;
	ng::math::Vec3 bottomPlaneNormal;

	ng::math::Vec3* position;

	Camera(float fov, float viewDistance, float htwRatio);
	~Camera();
};

