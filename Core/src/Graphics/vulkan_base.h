#pragma once

#include "../def.h"



namespace ng {
	namespace graphics {

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};
		
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
			std::vector<const char*> extensions;

			//Instance Layers
			std::vector<VkLayerProperties> installedLayers;
			std::vector<const char*> layers;
			
			VkInstance instance;

			//debug
			VkDebugReportFlagsEXT debugReportFlags;
			VkDebugReportCallbackEXT debugReportCallback;

			bool computeAndGraphicsSameDevice = false;
			
			VkSurfaceKHR surface;
			VkSurfaceFormatKHR surfaceFormat;
			VkFormat surfaceColorFormat;
			VkColorSpaceKHR surfaceColorSpace;

		public:

			void createInstance();
			void freeInstance();

			void createDebugCallback();
			void freeDebugCallback();

			void createLogicalDevices();
			void freeLogicalDevices();
			
			void createCommandPools();
			void freeCommandPools();

		};

	}
}

