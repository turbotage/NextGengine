#pragma once

#include "vulkan_base.h"

namespace ng {
	namespace graphics {

		class VulkanDevice
		{
		public:
			VkPhysicalDevice physicalDevice;

			VkPhysicalDeviceProperties properties;
			
			VkPhysicalDeviceFeatures features;

			VkPhysicalDeviceFeatures enabledFeatures;

			VkPhysicalDeviceMemoryProperties memoryProperties;

			std::vector<VkQueueFamilyProperties> queueFamilyProperties;
			
			std::vector<std::string> supportedExtensions;

			VkDevice logicalDevice;

			struct QueueFamilyIndices {
				uint32 graphics;
				uint32 compute;
				uint32 transfer;
			} queueFamilyIndices;

			bool debugMarkersEnabled = false;

			std::vector<VkThread> threads;

		public:

			operator VkDevice() { return logicalDevice; }

			VulkanDevice(VkPhysicalDevice physicalDevice);

			~VulkanDevice();

			uint32 getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr);

			uint32 getQueueFamilyIndex(VkQueueFlagBits queueFlags);

			VkResult createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
				std::vector<const char*> enabledExtensions,
				bool useSwapSchain = true,
				VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT
			);

			bool extensionSupported(std::string extension);

			VkCommandPool createCommandPool(uint32 queueFamilyIndex, 
				VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

			VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool commandPool, bool begin = false);

			void flushCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue, bool free = true);

			VkResult createBuffer(VkBufferUsageFlags usage,
				VkMemoryPropertyFlags memoryPropertyFlags,
				VkDeviceSize size,
				VkBuffer *buffer,
				VkDeviceMemory *memory,
				void *data = nullptr);



		};
		
	}
}



