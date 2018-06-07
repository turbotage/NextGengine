#pragma once

#include "../def.h"
#include "vulkan_tools.h"


namespace ng {
	namespace vulkan {
		
		class VkThread {
		public:
			uint32 queueFamilyIndex;
			VkCommandPool commandPool;
			std::vector<VkCommandBuffer> commandBuffers;
		};

		class VulkanBase {
		public:
			//Instance Extensions
			std::vector<VkExtensionProperties> installedExtensions;
			std::vector<const char*> enabledExtensions;

			//Instance Layers
			std::vector<VkLayerProperties> installedLayers;
			std::vector<const char*> enabledLayers;
			
			VkInstance instance;

			//debug
			VkDebugReportFlagsEXT debugReportFlags;
			VkDebugReportCallbackEXT debugReportCallback;

		public:

			void createInstance(std::vector<const char*> requiredExtensions);

			void createDebugCallback();

			VulkanBase();
			~VulkanBase();

		};

	}
}

