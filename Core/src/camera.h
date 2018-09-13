#pragma once

#include "plane.h"
#include "quaternion.h"

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

	ngm::Quaternion Rotation;
	ngm::Vec3f Position;

	ngm::Plane rightPlane;
	ngm::Plane leftPlane;
	ngm::Plane topPlane;
	ngm::Plane bottomPlane;

	ngm::Plane nearPlane;
	ngm::Plane farPlane;
	ngm::Plane cullPlane1;
	ngm::Plane	cullPlane2;

	ngm::Vec3f forward;
	ngm::Vec3f up;
	ngm::Vec3f right;

	ngm::Vec3f rightPlaneNormal;
	ngm::Vec3f leftPlaneNormal;
	ngm::Vec3f topPlaneNormal;
	ngm::Vec3f bottomPlaneNormal;

	Camera(float fov, float viewDistance, float htwRatio);
	~Camera();

	void update(ngm::Vec3f positionChange, ngm::Quaternion rotationChange);

};

