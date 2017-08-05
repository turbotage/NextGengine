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
			
			class VulkanBase {
				//Instance Extensions
				std::vector<vk::ExtensionProperties, std::allocator<vk::ExtensionProperties>> installedExtensions;
				std::vector<const char*> extensions;

				//Instance Layers
				std::vector<vk::LayerProperties, std::allocator<vk::LayerProperties>> installedLayers;
				std::vector<const char*> layers;

				vk::ApplicationInfo appInfo;

				vk::Instance instance;
				
				GraphicsUnit graphicsUnit;
				ComputeUnit computeUnit;

				vk::SurfaceFormatKHR surfaceFormat;
				vk::Format surfaceColorFormat;
				vk::ColorSpaceKHR surfaceColorSpace;

				vk::FormatProperties formatProperties;


			};

		}
	}
}

