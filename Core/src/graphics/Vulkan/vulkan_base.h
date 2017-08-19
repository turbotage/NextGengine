#pragma once

#include "../../def.h"
#include <vector>
#include "../window.h"

namespace ng {
	namespace graphics {
		
		struct PhysicalDevice {
			vk::PhysicalDevice device;
			vk::PhysicalDeviceMemoryProperties memoryProperties;
			std::vector<vk::ExtensionProperties, std::allocator<vk::ExtensionProperties>> deviceExtensions;
			std::vector<vk::LayerProperties, std::allocator<vk::LayerProperties>> deviceLayers;
			std::vector<const char*> validationLayers;
		};

		struct GraphicsUnit {
			PhysicalDevice* pDevice;
			vk::Device device;
			vk::Queue graphicsQueue;
			vk::DeviceCreateInfo createInfo;
		};

		struct ComputeUnit {
			PhysicalDevice* pDevice;
			vk::Device device;
			vk::Queue computeQueue;
			vk::DeviceCreateInfo createInfo;
		};
		
		struct Thread {
			vk::CommandPool commandPool;
			std::vector<vk::CommandBuffer> commandBuffers;
		};

		struct VulkanBase {
			//Instance Extensions
			std::vector<vk::ExtensionProperties, std::allocator<vk::ExtensionProperties>> installedExtensions;
			std::vector<const char*> extensions;

			//Instance Layers
			std::vector<vk::LayerProperties, std::allocator<vk::LayerProperties>> installedLayers;
			std::vector<const char*> layers;

			vk::Instance instance;

			//graphics
			GraphicsUnit graphicsUnit;
			std::vector<Thread> graphicsThreads;
			//computics
			ComputeUnit computeUnit;
			std::vector<Thread> computeThreads;

			vk::SurfaceFormatKHR surfaceFormat;
			vk::Format surfaceColorFormat;
			vk::ColorSpaceKHR surfaceColorSpace;

			vk::FormatProperties formatProperties;

			vk::Buffer staticGraphicsStorageBuffer;
			vk::DeviceMemory staticGraphicsBufferMemory;

			vk::Buffer staticComputeStorageBuffer;
			vk::DeviceMemory staticComputeBufferMemory;
			
			vk::DescriptorPool graphicsDescriptorPool;
			vk::DescriptorPool computeDescriptorPool;
			
			vk::ShaderModule vertexModule;
			vk::ShaderModule fragmentModule;
			vk::ShaderModule computeModule;
			
			vk::Pipeline graphicsPipeline;
			vk::PipelineCache graphicsPipelineCache;

			vk::Pipeline computePipeline;
			vk::PipelineCache computePipelineCache;

			vk::RenderPass renderPass;
			
			vk::Framebuffer framebuffer;

		};

	}
}

