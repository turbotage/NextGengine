#pragma once

#include <GLFW\glfw3.h>
#include <vulkan\vulkan.h>
#include "../../def.h"
#include <vector>

namespace ng {
	namespace graphics {
		namespace vulkan {

			struct LayerProperties {
				VkLayerProperties properties;
				std::vector<VkExtensionProperties> extensions;
			};

			struct GPU {
				VkPhysicalDevice physicalDevice;
				VkPhysicalDeviceProperties properties;
				VkPhysicalDeviceMemoryProperties memoryProperties;
				VkQueueFamilyProperties queueProperties;
			};

			struct VulkanBase {

				GLFWwindow* window;
				VkSurfaceKHR surface;

				bool useStagingBuffer;

				std::vector<const char*> instanceLayerNames;
				std::vector<const char*> instanceExtensionNames;
				std::vector<LayerProperties> instanceLayerProperties;
				std::vector<VkExtensionProperties> instanceExtensionProperties;
				VkInstance instance;
				
				std::vector<const char*> deviceExtensionNames;
				std::vector<VkExtensionProperties> deviceExtensionProperties;

				std::vector<GPU> gpus;
				

				VkDevice device;
				
				VkQueue graphicsQueue;
				VkQueue presentQueue;
				VkQueue computeQueue;

				uint graphicsQueue;

				VkFramebuffer* framebuffers;
				

			};

		}
	}
}

