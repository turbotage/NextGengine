#include "camera_node.h"

ng::scenegraph::CameraNode::CameraNode(float fov, float viewDistance, float htwRatio)
	: fov(fov), farClipDistance(viewDistance)
{

	float r = farClipDistance / cos(fov);
	halfWidth = sin(fov) * r;
	halfHeight = halfWidth * htwRatio;
	forward = ng::math::Vec3f(0.0f, 0.0f, 1.0f);
	up = ng::math::Vec3f(0.0f, 1.0f, 0.0f);
	right = ng::math::Vec3f(1.0f, 0.0f, 0.0f);

	ng::math::Vec3f temp;
	temp = (farClipDistance * forward);
	rightPlaneNormal = up.cross(temp + halfWidth * right);
	leftPlaneNormal = (temp - halfWidth * right).cross(up);
	topPlaneNormal = (temp - halfHeight * up).cross(right);
	bottomPlaneNormal = right.cross(temp + halfHeight * up);

	farPlane.setPlane(forward, m_Position + (farClipDistance * forward));
	nearPlane.setPlane(forward, m_Position + (nearClipDistance * forward));
	rightPlane.setPlane(rightPlaneNormal, m_Position);
	leftPlane.setPlane(leftPlaneNormal, m_Position);
	topPlane.setPlane(topPlaneNormal, m_Position);
	bottomPlane.setPlane(bottomPlaneNormal, m_Position);
}

void ng::scenegraph::CameraNode::onUpdate(float time)
{
	//TOTO the newRotation should be set by user input
	ng::math::Quaternion newRotation;

	newRotation.rotate4(forward, up, right, ng::math::Vec3f(0.0f,0.0f,0.0f));
	newRotation.rotate4(rightPlaneNormal, leftPlaneNormal, topPlaneNormal, bottomPlaneNormal);

	farPlane.setPlane(forward, m_Position + (farClipDistance * forward));
	nearPlane.setPlane(forward, m_Position + (nearClipDistance * forward));
	rightPlane.setPlane(rightPlaneNormal, m_Position);
	leftPlane.setPlane(leftPlaneNormal, m_Position);
	topPlane.setPlane(topPlaneNormal, m_Position);
	bottomPlane.setPlane(bottomPlaneNormal, m_Position);

}
