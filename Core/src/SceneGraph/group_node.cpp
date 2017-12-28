#include "group_node.h"


void ng::scenegraph::GroupNode::setCalculatedCombinedCenter()
{
	m_CenterPosition = ng::math::Vec3(0.0f, 0.0f, 0.0f);

	ng::math::Vec3 dirVec;
	for (int i = 0; i < m_Children.size(); ++i) {
		 dirVec = (m_Children[i]->getCenterPosition());
	}

	m_CenterPosition /= (float)m_Children.size();
}

void ng::scenegraph::GroupNode::addChild(SceneNode* child) 
{
	m_Children.push_back(child);
	setCalculatedCombinedCenter();
}
