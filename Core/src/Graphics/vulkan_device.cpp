#include "vulkan_device.h"

#include "../debug.h"

ng::graphics::VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice, std::vector<VkThread>* threads) 
{
	assert(physicalDevice);

	assert(threads);

	this->threads = threads;

	this->physicalDevice = physicalDevice;

	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	vkGetPhysicalDeviceFeatures(physicalDevice, &features);

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	uint32 queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	
	assert(queueFamilyCount > 0);
	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	uint32 extensionsCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);

	if (extensionsCount > 0) {
		std::vector<VkExtensionProperties> extensions(extensionsCount);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, &extensions.front()) == VK_SUCCESS) {
			for (auto extension : extensions) {
				supportedExtensions.push_back(extension.extensionName);
			}
		}
	}

}

ng::graphics::VulkanDevice::~VulkanDevice() 
{
	for (auto& thread : *threads) {
		if (thread.commandPool) {
			vkDestroyCommandPool(logicalDevice, thread.commandPool, nullptr);
		}
	}
	if (logicalDevice) {
		vkDestroyDevice(logicalDevice, nullptr);
	}
}

uint32 ng::graphics::VulkanDevice::getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr)
{
	for (uint32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
		if ((typeBits & 1) == 1) {
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				if (memTypeFound) {
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}
	if (memTypeFound) {
		*memTypeFound = false;
		return 0;
	}
	else {
		throw std::runtime_error("Could not find a matching memory type");
	}
}

uint32 ng::graphics::VulkanDevice::getQueueFamilyIndex(VkQueueFlagBits queueFlags) 
{
	if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
		for (uint32 i = 0; i < static_cast<uint32>(queueFamilyProperties.size()); ++i) {
			if ((queueFamilyProperties[i].queueFlags & queueFlags) && 
				(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
				return i;
				break;
			}
		}
	}

	if (queueFlags & VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32 i = 0; i < static_cast<uint32>(queueFamilyProperties.size()); ++i) {
			if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
				((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
				((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {

				return i;
				break;
			}
		}
	}

	for (uint32 i = 0; i < static_cast<uint32>(queueFamilyProperties.size()); ++i) {
		if (queueFamilyProperties[i].queueFlags & queueFlags) {
			return i;
			break;
		}
	}

	throw std::runtime_error("Could not find a matching queue family index");

}

VkResult ng::graphics::VulkanDevice::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, 
	std::vector<const char*> enabledExtensions, 
	bool useSwapSchain, 
	VkQueueFlags requestedQueueTypes)
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	const float defaultQueuePriority = 0.0f;

	//Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
		queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);
	}
	else {
		queueFamilyIndices.graphics = VK_NULL_HANDLE;
	}

	//Dedicated compute queue
	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
		queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
		if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	}
	else {
		queueFamilyIndices.compute = queueFamilyIndices.graphics;
	}

	//Dedicated transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
		queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
		if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute)) {
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
			queueInfo.queueCount;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	}
	else {
		queueFamilyIndices.transfer = queueFamilyIndices.graphics;
	}

	std::vector<const char*> deviceExtensions(enabledExtensions);
	if (useSwapSchain) {
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

	if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
		deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		debugMarkersEnabled = true;
	}

	if (deviceExtensions.size() > 0) {
		deviceCreateInfo.enabledExtensionCount = (uint32)deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	}

	VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);

	if (result == VK_SUCCESS) {
		for (auto& thread : *threads) {
			thread.commandPool = createCommandPool(thread.queueFamilyIndex);
		}
	}

	this->enabledFeatures = enabledFeatures;

	return result;
}

bool ng::graphics::VulkanDevice::extensionSupported(std::string extension)
{
	return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
}

VkCommandPool ng::graphics::VulkanDevice::createCommandPool(uint32 queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
	commandPoolInfo.flags = createFlags;
	VkCommandPool cmdPool;
	VULKAN_CHECK_RESULT(vkCreateCommandPool(logicalDevice, &commandPoolInfo, nullptr, &cmdPool));
	return cmdPool;
}

VkCommandBuffer ng::graphics::VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool commandPool, bool begin)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VULKAN_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, &commandBuffer));

	if (begin) {
		VkCommandBufferBeginInfo commandBufferInfo;
		VULKAN_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferInfo));
	}

	return commandBuffer;
}

void ng::graphics::VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue, bool free)
{
	if (commandBuffer == VK_NULL_HANDLE) {
		return;
	}

	VULKAN_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFenceCreateInfo fenceInfo = {};
	VkFence fence;
	VULKAN_CHECK_RESULT(vkCreateFence(logicalDevice, &fenceInfo, nullptr, &fence));

	VULKAN_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

	vkDestroyFence(logicalDevice, fence, nullptr);

	if (free) {
		vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	}
}

VkResult ng::graphics::VulkanDevice::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer * buffer, VkDeviceMemory * memory, void * data)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.size = size;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VULKAN_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer));

	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, memoryPropertyFlags);
	VULKAN_CHECK_RESULT(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, memory));

	if (data != nullptr) {
		void* mapped;
		VULKAN_CHECK_RESULT(vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped));
		memcpy(mapped, data, size);

		if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = *memory;
			mappedRange.offset = 0;
			mappedRange.size = size;
			vkFlushMappedMemoryRanges(logicalDevice, 1, &mappedRange);
		}

		vkUnmapMemory(logicalDevice, *memory);
	}

	VULKAN_CHECK_RESULT(vkBindBufferMemory(logicalDevice, *buffer, *memory, 0));

	return VK_SUCCESS;
}

