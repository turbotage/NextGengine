#pragma once

#include "pch.h"

namespace ngv {

	/// Factory for instances.
	class VulkanInstanceMaker {
	public:
		VulkanInstanceMaker() {};

		/// Set the default layers and extensions.
		VulkanInstanceMaker& setDefaultLayers();
		
		/// Add a layer. eg. "VK_LAYER_LUNARG_standard_validation"
		VulkanInstanceMaker& addLayer(const char* layer);

		/// Add an extension. eg. VK_EXT_DEBUG_REPORT_EXTENSION_NAME
		VulkanInstanceMaker& addExtension(const char* layer);

		/// Set the name of the application.
		VulkanInstanceMaker& setApplicationName(const char* pApplicationName);

		/// Set the version of the application.
		VulkanInstanceMaker& setApplicationVersion(uint32 applicationVersion);

		/// Set the name of the engine.
		VulkanInstanceMaker& setEngineName(const char* pEngineName);

		/// Set the version of the engine.
		VulkanInstanceMaker& setEngineVersion(uint32 engineVersion);

		/// Set the version of the api.
		VulkanInstanceMaker& setApiVersion(uint32 apiVersion);

		/// Create a self-deleting (unique) instance.
		vk::UniqueInstance createUnique();

	private:
		std::vector<const char*> m_Layers;
		std::vector<const char*> m_Extensions;
		vk::ApplicationInfo m_AppInfo;
	};


}