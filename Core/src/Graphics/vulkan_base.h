#pragma once

#include "../def.h"



namespace ng {
	namespace graphics {
		
		enum {
			GRAPHICS_UNIT,
			COMPUTE_UNIT,
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		struct QueueFamilyIndices {
			int graphicsFamily = -1;
			int presentFamily = -1;
			int computeFamily = -1;

			bool isGraphicsComplete() {
				return graphicsFamily >= 0;
			}

			bool isComputeComplete() {
				return computeFamily >= 0;
			}

			bool isPresentComplete() {
				return presentFamily >= 0;
			}
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

			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

			void createInstance();
			void freeInstance();

			void createDebugCallback();
			void freeDebugCallback();

			void createPhysicalDevices();
			//void freePhysicalDevices();

			void createLogicalDevices();
			void freeLogicalDevices();
			
			void createCommandPools();
			void freeCommandPools();

		};

	}
}

