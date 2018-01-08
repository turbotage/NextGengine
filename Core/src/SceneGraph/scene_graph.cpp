#include "scene_graph.h"

void ng::scenegraph::SceneNode::updateBoundingVolumes()
{
	if (m_Parent == nullptr) {
		return;
	}
	float distAndRadius = (m_BoundingSphere.centerPos - m_Parent->getBoundingSpherePosition()).norm() + m_BoundingSphere.radius;
	if (distAndRadius > m_Parent->getBoundingSphereRadius()) {
		m_Parent->setBoundingSphereRadius(distAndRadius);
		return m_Parent->updateBoundingVolumes();
	}
}

ng::scenegraph::SceneNode::SceneNode()
{

}

const ng::math::Vec3f& ng::scenegraph::SceneNode::getCenterPosition()
{
	return m_Position;
}

const ng::math::Vec3f& ng::scenegraph::SceneNode::getBoundingSpherePosition()
{
	return m_BoundingSphere.centerPos;
}

float ng::scenegraph::SceneNode::getBoundingSphereRadius() const
{
	return m_BoundingSphere.radius;
}

void ng::scenegraph::SceneNode::setBoundingSphereRadius(float radius)
{
	m_BoundingSphere.radius = radius;
}

bool ng::scenegraph::SceneNode::hasMovementProperties(ng::props::mMovementPropertiesMask props)
{
	return (m_MovementProperties & props);
}

void ng::scenegraph::SceneNode::addMovementProperties(ng::props::mMovementPropertiesMask props)
{
	m_MovementProperties = (m_MovementProperties | props);
}

void ng::scenegraph::SceneNode::setMovementProperties(ng::props::mMovementPropertiesMask props)
{
	m_MovementProperties = props;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::rotate(const ng::math::Vec3f & rotationAxis, const float angle)
{
	m_WorldTransform *= ng::math::Mat4::rotation(rotationAxis, angle);
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::rotate(const ng::math::Mat4 & rotationMatrix)
{
	m_WorldTransform *= rotationMatrix;
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::rotate(const ng::math::Quaternion & rotationQuaternion)
{
	m_WorldTransform *= ng::math::Mat4::rotation(rotationQuaternion);
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::rotateAround(const ng::math::Vec3f & rotationPoint, const ng::math::Vec3f & rotationAxis, const float angle)
{
	// T(x, y, z) * R * T(-x, -y, -z)
	m_WorldTransform *= ng::math::Mat4::translation(rotationPoint);
	m_WorldTransform *= ng::math::Mat4::rotation(rotationAxis, angle);
	m_WorldTransform *= ng::math::Mat4::translation(-1.0f*rotationPoint);
	updateBoundingVolumes();
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::translate(const ng::math::Vec3f & translation)
{
	m_WorldTransform *= ng::math::Mat4::translation(translation);
	updateBoundingVolumes();
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::translate(const ng::math::Mat4 & translationMatrix)
{
	m_WorldTransform *= translationMatrix;
	updateBoundingVolumes();
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::transform(const ng::math::Mat4 & transformation)
{
	m_WorldTransform *= transformation;
	updateBoundingVolumes();
	return m_WorldTransform;
}

void ng::scenegraph::SceneNode::addChild(SceneNode * childNode)
{
	m_Children.push_back(childNode);
	onAddChild();
}

void ng::scenegraph::SceneNode::update(float time)
{

	onAddChild();
}
