#include "render_state.h"

std::size_t ng::scenegraph::RenderState::hash(RenderState const & renderState)
{
	return ng::graphics::VulkanModel::hash(renderState.model);
}

std::size_t ng::scenegraph::RenderState::hash(std::list<RenderState>::iterator iter)
{
	return iter->hash(*iter);
}
