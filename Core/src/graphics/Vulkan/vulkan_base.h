#pragma once

#include <vulkan\vulkan.hpp>
#include "../../def.h"
#include <vector>

namespace ng {
	namespace graphics {
		namespace vulkan {
			
			struct PhysicalDevice {
				vk::PhysicalDevice device;
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

			class VulkanBase {
				//Instance Extensions
				std::vector<vk::ExtensionProperties, std::allocator<vk::ExtensionProperties>> installedExtensions;
				std::vector<const char*> extensions;

				//Instance Layers
				std::vector<vk::LayerProperties, std::allocator<vk::LayerProperties>> installedLayers;
				std::vector<const char*> layers;

				vk::ApplicationInfo appInfo;

				vk::Instance instance;
				
				//graphics
				GraphicsUnit graphicsUnit;
				vk::Queue graphicsQueue;
				std::vector<Thread> graphicsThreads;
				//computics
				ComputeUnit computeUnit;
				vk::Queue computeQueue;
				std::vector<Thread> computeThreads;

				vk::SurfaceFormatKHR surfaceFormat;
				vk::Format surfaceColorFormat;
				vk::ColorSpaceKHR surfaceColorSpace;

				vk::FormatProperties formatProperties;

				VkBuffer vertexBuffer;
				VkDeviceMemory vertexBufferMemory;
				
				VkBuffer indexBuffer;
				VkDeviceMemory indexBufferMemory;

				VkBuffer uniformBuffer;
				VkDeviceMemory uniformBufferMemory;

				VkDescriptorPool descriptorPool;
				VkDescriptorSet descriptorSet;


			};

		}
	}
}

