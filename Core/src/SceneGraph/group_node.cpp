#include "group_node.h"


void ng::scenegraph::GroupNode::setCalculatedCombinedCenter()
{
	m_BoundingSphere.centerPos = ng::math::Vec3f(0.0f, 0.0f, 0.0f);

	ng::math::Vec3f dirVec;
	for (int i = 0; i < m_Children.size(); ++i) {
		 m_BoundingSphere.centerPos += (m_Children[i]->getCenterPosition() * m_Children[i]->getBoundingSphereRadius());
	}

	m_BoundingSphere.centerPos /= (float)m_Children.size();
}

void ng::scenegraph::GroupNode::addChild(SceneNode* child) 
{
	m_Children.push_back(child);
	setCalculatedCombinedCenter();
}
