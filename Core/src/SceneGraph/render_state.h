#pragma once

#include "../def.h"

#include "../Vulkan/GraphicsObjects/vulkan_model.h"


namespace ng {
	namespace scenegraph {

		class RenderState {
		private:
			uint32 m_Instances;
		public:

			/**  VULKAN-MODEL : vulkan-model  **/
			ng::vulkan::VulkanModel model;

		public:

			uint32 getInstances();
			
			static std::size_t hash(RenderState const& renderState);

			static std::size_t hash(std::list<RenderState>::iterator iter);

		};

	}
}

namespace std {

	template<> struct hash<ng::scenegraph::RenderState> {
		size_t operator()(ng::scenegraph::RenderState const& renderState) const {
			return ng::scenegraph::RenderState::hash(renderState);
		}
	};

	template<> struct hash<std::list<ng::scenegraph::RenderState>::iterator> {
		size_t operator()(std::list<ng::scenegraph::RenderState>::iterator const& iter) const {
			return ng::scenegraph::RenderState::hash(iter);
		}
	};

}