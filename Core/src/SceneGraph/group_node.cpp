#include "group_node.h"


void ng::scenegraph::GroupNode::calcCombinedCenter()
{
	
	m_CenterPosition = ng::math::Vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < m_Children.size(); ++i) {
		m_CenterPosition += m_Children[i]->m_CenterPosition;
	}

}

void ng::scenegraph::GroupNode::addChild(SceneNode* child) {

}
