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

	ng::math::Vec3f forward;
	ng::math::Vec3f up;
	ng::math::Vec3f right;

	ng::math::Vec3f rightPlaneNormal;
	ng::math::Vec3f leftPlaneNormal;
	ng::math::Vec3f topPlaneNormal;
	ng::math::Vec3f bottomPlaneNormal;

	ng::math::Vec3f* position;

	Camera(float fov, float viewDistance, float htwRatio);
	~Camera();
};

