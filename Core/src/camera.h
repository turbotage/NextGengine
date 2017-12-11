#pragma once

#include "Math\vec3.h"
#include "Math\quaternion.h"

class Camera
{
private:



	bool isInView();

public:

	float m_FieldOfView;

	float m_FarPlaneDistance;
	float m_NearPlaneDistance;
	float m_HalfWidth;
	float m_HalfHeight;

	ng::math::Quaternion m_Rotation;

	ng::math::Vec3 m_Forward;
	ng::math::Vec3 m_Up;
	ng::math::Vec3 m_Right;
	ng::math::Vec3 m_Left;

	Camera(float fov, float viewDistance, float htwRatio);
	~Camera();
};

