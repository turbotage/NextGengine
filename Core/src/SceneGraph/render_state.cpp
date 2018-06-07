#include "render_state.h"

uint32 ng::scenegraph::RenderState::getInstances()
{
	return m_Instances;
}

std::size_t ng::scenegraph::RenderState::hash(RenderState const & renderState)
{
	return ng::vulkan::VulkanModel::hash(renderState.model);
}

std::size_t ng::scenegraph::RenderState::hash(std::list<RenderState>::iterator iter)
{
	return iter->hash(*iter);
}
