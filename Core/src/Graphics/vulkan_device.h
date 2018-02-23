#pragma once

#include "vulkan_base.h"

#define VULKAN_DEVICE_ID_PRESENT_UNIT 0
#define VULKAN_DEVICE_ID_COMPUTE_UNIT 1

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

				bool isGraphicsComplete() { return graphics >= 0; }

				bool isComputeComplete() { return compute >= 0; }

				bool isTransferComplete() { return transfer >= 0; }

			} queueFamilyIndices;

			bool debugMarkersEnabled = false;

			uint16 deviceID;

			std::vector<VkThread>* threads;

		public:

			operator VkDevice() { return logicalDevice; }

			VulkanDevice(VkPhysicalDevice physicalDevice, std::vector<VkThread>* threads);

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



