#include "renderable_node.h"

ng::scenegraph::RenderableNode * ng::scenegraph::RenderableNode::cast(SceneNode * node)
{
	return static_cast<RenderableNode*>(node);
}

ng::scenegraph::InstanceData ng::scenegraph::RenderableNode::getInstanceData()
{
	InstanceData ret;
	ret.transformation = m_WorldTransform;
	ret.textureArrayIndex = m_TextureArrayIndex;
	return ret;
}
