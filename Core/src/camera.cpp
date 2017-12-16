#include "camera.h"



bool Camera::isInView()
{
	
}


Camera::Camera(float fov, float viewDistance, float htwRatio)
	: fieldOfView(fov), farPlaneDistance(viewDistance)
{
	float r = farPlaneDistance / cos(fieldOfView);
	halfWidth = sin(fieldOfView) * r;
	halfHeight = halfWidth * htwRatio;
	forward = ng::math::Vec3(0.0f, 0.0f, 1.0f);
	up = ng::math::Vec3(0.0f, 1.0f, 0.0f);
	right = ng::math::Vec3(1.0f, 0.0f, 0.0f);

	ng::math::Vec3 temp;
	temp = (farPlaneDistance * forward);
	rightPlaneNormal = up.cross(temp + halfWidth*right);
	leftPlaneNormal = (temp - halfWidth * right).cross(up);
	topPlaneNormal = (temp - halfHeight * up).cross(right);
	bottomPlaneNormal = right.cross(temp + halfHeight * up);

}

Camera::~Camera()
{

}
