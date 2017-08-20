#pragma once

#include "../../def.h"
#include <vector>
#include "../window.h"

namespace ng {
	namespace graphics {
		
		struct QueueFamilyIndices {
			int graphicsFamily = -1;
			int presentFamily = -1;

			bool isComplete() {
				return graphicsFamily >= 0 && presentFamily >= 0;
			}
		};

		struct PhysicalDevice {
			VkPhysicalDevice device;
			VkPhysicalDeviceMemoryProperties memoryProperties;
			std::vector<VkExtensionProperties> deviceExtensions;
			std::vector<VkLayerProperties> deviceLayers;
			std::vector<const char*> validationLayers;
		};

		struct GraphicsUnit {
			PhysicalDevice* pDevice;
			VkDevice device;
			VkQueue graphicsQueue;
			VkDeviceCreateInfo createInfo;
		};

		struct ComputeUnit {
			PhysicalDevice* pDevice;
			VkDevice device;
			VkQueue computeQueue;
			VkDeviceCreateInfo createInfo;
		};
		
		struct Thread {
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
			VkDebugReportCallbackEXT callback;

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

			VkFormatProperties formatProperties;

			
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

			void createInstance();

			void createDebugCallback();

			void createDevices();



		};

	}
}

