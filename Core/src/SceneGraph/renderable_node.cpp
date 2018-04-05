#include "renderable_node.h"

ng::scenegraph::InstanceData ng::scenegraph::RenderableNode::getInstanceData()
{
	InstanceData ret;
	ret.transformation = m_WorldTransform;
	ret.textureArrayIndex = m_TextureArrayIndex;
	return ret;
}
