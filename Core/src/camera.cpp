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
	forward = ngm::Vec3f(0.0f, 0.0f, 1.0f);
	up = ngm::Vec3f(0.0f, 1.0f, 0.0f);
	right = ngm::Vec3f(1.0f, 0.0f, 0.0f);

	ngm::Vec3f temp;
	temp = (farPlaneDistance * forward);
	rightPlaneNormal = up.cross(temp + halfWidth*right);
	leftPlaneNormal = (temp - halfWidth * right).cross(up);
	topPlaneNormal = (temp - halfHeight * up).cross(right);
	bottomPlaneNormal = right.cross(temp + halfHeight * up);

}

Camera::~Camera()
{

}

void Camera::update(ngm::Vec3f positionChange, ngm::Quaternion rotationChange)
{
	Position += positionChange;
	Rotation *= rotationChange;

	

}
