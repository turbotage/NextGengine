#include "camera.h"



bool ng::Camera::isInView()
{
	return true;
}

ng::Camera::Camera()
{
}

ng::Camera::Camera(CameraCreateInfo createInfo)
{
	init(createInfo);
}

void ng::Camera::init(CameraCreateInfo createInfo)
{
	fieldOfView = createInfo.fov;
	farPlaneDistance = createInfo.viewDistance;

	float r = farPlaneDistance / cos(fieldOfView);
	halfWidth = sin(fieldOfView) * r;
	halfHeight = halfWidth * createInfo.htwRatio;
	forward = ngm::Vec3f(0.0f, 0.0f, 1.0f);
	up = ngm::Vec3f(0.0f, 1.0f, 0.0f);
	right = ngm::Vec3f(1.0f, 0.0f, 0.0f);

	ngm::Vec3f temp;
	temp = (farPlaneDistance * forward);
	rightPlaneNormal = up.cross(temp + halfWidth * right);
	leftPlaneNormal = (temp - halfWidth * right).cross(up);
	topPlaneNormal = (temp - halfHeight * up).cross(right);
	bottomPlaneNormal = right.cross(temp + halfHeight * up);

	position += createInfo.startTranslation;
	rotation *= createInfo.startRotation;


}

ng::Camera::~Camera()
{

}

void ng::Camera::update(ngm::Vec3f positionChange, ngm::Quaternion rotationChange)
{
	position += positionChange;
	rotation *= rotationChange;

	

}
