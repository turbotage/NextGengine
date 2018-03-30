#pragma once

#include "vulkan_base.h"

#define VULKAN_DEVICE_ID_PRESENT_UNIT 0
#define VULKAN_DEVICE_ID_COMPUTE_UNIT 1

namespace ng {
	namespace graphics {

		//standard device-extensions

		const std::vector<const char*> graphicsDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const std::vector<const char*> computeDeviceExtensions = {

		};

		enum VulkanDeviceTypeBits {
			VULKAN_DEVICE_TYPE_DESCRETE_GRAPHICS_UNIT = 1,
			VULKAN_DEVICE_TYPE_DESCRETE_COMPUTE_UNIT = 2,
			VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT = 4,
			VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT_IN_GRAPHICS_QUEUE = 8
		};

		typedef bool VulkanDeviceTypeFlags;

		struct QueueFamilyIndices {
			int32 graphics = -1;
			int32 compute = -1;
			int32 transfer = -1;
			int32 present = -1;

			bool isGraphicsComplete() { return graphics >= 0; }

			bool isComputeComplete() { return compute >= 0; }

			bool isTransferComplete() { return transfer >= 0; }

			bool isPresentComplete() { return present >= 0; }

			static bool isSame(int32 queue1, int32 queue2) { return (queue1 == queue2); }

		};

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

			std::vector<std::string> enabledExtensions;

			VkDevice logicalDevice;

			QueueFamilyIndices queueFamilyIndices;

			bool debugMarkersEnabled = false;

			uint16 deviceID;



		public:

			operator VkDevice() { return logicalDevice; }

			VulkanDevice();
			VulkanDevice(VkPhysicalDevice physicalDevice);
			void init(VkPhysicalDevice physicalDevice);

			~VulkanDevice();

			uint32 getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr);

			int32 getQueueFamilyIndex(VkQueueFlagBits queueFlags);

			std::pair<int32, int32> getGraphicsAndPresentQueueFamilyIndex(VkSurfaceKHR surface);

			void createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
				std::vector<const char*> enabledExtensions,
				VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
				VkSurfaceKHR surface = nullptr
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

			uint32 getMemoryScore();

			uint32 getDeviceScore(VulkanDeviceTypeFlags deviceTypeFlags, 
				std::vector<const char*> requiredExtentions, 
				VkSurfaceKHR surface
			);

		};
		
	}
}



