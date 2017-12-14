#include "camera.h"



bool Camera::isInView()
{
	
}


Camera::Camera(float fov, float viewDistance, float htwRatio)
	: m_FieldOfView(fov), m_FarPlaneDistance(viewDistance)
{
	float r = m_FarPlaneDistance / cos(m_FieldOfView);
	m_HalfWidth = sin(m_FieldOfView) * r;
	m_HalfHeight = m_HalfWidth * htwRatio;
	m_Forward = ng::math::Vec3(0.0f, 0.0f, 1.0f);
	m_Up = ng::math::Vec3(0.0f, 1.0f, 0.0f);
	m_Right = ng::math::Vec3(1.0f, 0.0f, 0.0f);
	m_Left = ng::math::Vec3(-1.0f, 0.0f, 0.0f);
}

Camera::~Camera()
{

}
