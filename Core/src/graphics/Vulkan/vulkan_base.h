#pragma once

#include "../../def.h"
#include <vector>



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

		struct PhysicalDevice {
			VkPhysicalDevice device = VK_NULL_HANDLE;
			VkPhysicalDeviceMemoryProperties memoryProperties;
			std::vector<VkExtensionProperties> deviceExtensions;
			std::vector<VkLayerProperties> deviceLayers;
		};

		struct GraphicsUnit {
			PhysicalDevice pDevice;
			VkDevice device;
			VkQueue graphicsQueue;
			VkQueue presentQueue;
			VkDeviceCreateInfo createInfo;
		};

		struct ComputeUnit {
			PhysicalDevice pDevice;
			VkDevice device;
			VkQueue computeQueue;
			VkDeviceCreateInfo createInfo;
		};
		
		struct Thread {
			VkCommandPool commandPool;
			std::vector<VkCommandBuffer> commandBuffers;
		};

		struct VulkanBase {
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
			//graphics
			GraphicsUnit graphicsUnit;
			std::vector<Thread> graphicsThreads;
			//computics
			ComputeUnit computeUnit;
			std::vector<Thread> computeThreads;
			
			VkSurfaceKHR surface;
			VkSurfaceFormatKHR surfaceFormat;
			VkFormat surfaceColorFormat;
			VkColorSpaceKHR surfaceColorSpace;

			//VkFormatProperties formatProperties;

			VkDescriptorPool graphicsDescriptorPool;
			VkDescriptorPool computeDescriptorPool;
			
			VkShaderModule vertexModule;
			VkShaderModule fragmentModule;
			VkShaderModule computeModule;
			
			VkPipeline graphicsPipeline;
			VkPipelineCache graphicsPipelineCache;

			VkPipeline computePipeline;
			VkPipelineCache computePipelineCache;

			VkRenderPass renderPass;
			
			VkFramebuffer framebuffer;

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

		};

	}
}

