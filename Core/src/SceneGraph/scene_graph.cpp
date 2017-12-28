#include "scene_graph.h"

ng::math::Vec3 ng::scenegraph::SceneNode::getCenterPosition()
{
	return m_CenterPosition;
}

float ng::scenegraph::SceneNode::getBoundingSphereRadius()
{
	return m_BoundingSphereRadius;
}

void ng::scenegraph::SceneNode::setBoundingSphereRadius(float radius)
{
	m_BoundingSphereRadius = radius;
}
