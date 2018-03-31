#pragma once

#include "scene_graph.h"
#include "../Graphics/GraphicsObjects/vulkan_model.h"

namespace ng {
	namespace scenegraph {

		struct InstanceData {
			ng::math::Mat4f transformation;
			uint32 textureArrayIndex;
		};

		class RenderableNode : SceneNode
		{
		private:

			/**  RENDER-OBJECT  **/
			ng::graphics::VulkanModel model;

		public:

			/**  set to false or true to enable or disable rendering  **/
			bool shouldBeRendered;

			/**  BOUNDING-SPHERE  **/
			ng::bvolumes::BoundingSphere boundingSphere;

			RenderableNode();
			~RenderableNode();

		};


	}
}
