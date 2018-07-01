#include "vulkan_device.h"
#include "vulkan_swapchain.h"

#include "../debug.h"

ng::vulkan::VulkanDevice::VulkanDevice()
{
}

ng::vulkan::VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice)
{
	init(physicalDevice);
}

void ng::vulkan::VulkanDevice::init(VkPhysicalDevice physicalDevice)
{
	assert(physicalDevice);

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

void ng::vulkan::VulkanDevice::cleanup()
{
	m_CleanupCalled = true;
	if (logicalDevice != VK_NULL_HANDLE) {
		vkDestroyDevice(logicalDevice, nullptr);
	}
}

ng::vulkan::VulkanDevice::~VulkanDevice()
{
	if (!m_CleanupCalled) {
		cleanup();
	}
}

int32 ng::vulkan::VulkanDevice::getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties)
{
	for (uint32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
		if ((typeBits & 1) == 1) {
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		typeBits >>= 1;
	}
	LOGD("no found matching memory type");
	return -1;
}

int32 ng::vulkan::VulkanDevice::getQueueFamilyIndex(VkQueueFlagBits queueFlags) 
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

	LOGD("no queue family could meet the specified requirements");
	return -1;
	//throw std::runtime_error("could not find queue family");
}

std::pair<int32, int32> ng::vulkan::VulkanDevice::getGraphicsAndPresentQueueFamilyIndex(VkSurfaceKHR surface)
{
	std::pair<int32, int32> ret(-1, -1);
	if (surface != nullptr) {
		for (uint32 i = 0; i < static_cast<uint32>(queueFamilyProperties.size()); ++i) {
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				VkBool32 presentSupport;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
				if (presentSupport) {
					ret.first = i;
					ret.second = i;
					return ret;
				}
			}
		}
		for (uint32 i = 0; i < static_cast<uint32>(queueFamilyProperties.size()); ++i) {
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				ret.first = i;
			}
			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
			if (presentSupport) {
				ret.second = i;
			}
			if ((ret.first != -1) && (ret.second != -1)) {
				return ret;
			}
		}
	}
	else {
		for (uint32 i = 0; i < static_cast<uint32>(queueFamilyProperties.size()); ++i) {
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				ret.first = i;
				ret.second = -1;
				return ret;
			}
		}
	}

	LOGD("no queue family could meet the specified requirements");
	return ret;
}

void ng::vulkan::VulkanDevice::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
	std::vector<const char*> enabledExtensions,
	VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
	VkSurfaceKHR surface = nullptr
)
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	const float defaultQueuePriority = 0.0f;

	//Queue-Creation
	{
		std::pair<int32, int32> graphicsAndPresentQueue;
		//Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
			graphicsAndPresentQueue = getGraphicsAndPresentQueueFamilyIndex(surface);
			queueFamilyIndices.graphics = graphicsAndPresentQueue.first;
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);

			if (surface != nullptr) {
				queueFamilyIndices.present = graphicsAndPresentQueue.second;
				VkDeviceQueueCreateInfo queueInfo2 = {};
				queueInfo2.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo2.queueFamilyIndex = graphicsAndPresentQueue.second;
				queueInfo2.queueCount = 1;
				queueInfo2.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo2);
			}
			else {
				queueFamilyIndices.present = VK_NULL_HANDLE;
			}

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
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

	// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
	if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
		enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		debugMarkersEnabled = true;
	}

	if (enabledExtensions.size() > 0) {
		deviceCreateInfo.enabledExtensionCount = (uint32)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}

	VULKAN_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice));

	this->enabledFeatures = enabledFeatures;

	for (auto ext : enabledExtensions) {
		this->enabledExtensions.push_back(std::string(ext));
	}
}

bool ng::vulkan::VulkanDevice::extensionSupported(std::string extension)
{
	return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
}

VkCommandPool ng::vulkan::VulkanDevice::createCommandPool(uint32 queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
	commandPoolInfo.flags = createFlags;
	VkCommandPool cmdPool;
	VULKAN_CHECK_RESULT(vkCreateCommandPool(logicalDevice, &commandPoolInfo, nullptr, &cmdPool));
	return cmdPool;
}

VkCommandBuffer ng::vulkan::VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool commandPool, bool begin = false)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = level;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VULKAN_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, &commandBuffer));

	if (begin) {
		VkCommandBufferBeginInfo commandBufferInfo;
		VULKAN_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferInfo));
	}

	return commandBuffer;
}

void ng::vulkan::VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue, bool free)
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

VkResult ng::vulkan::VulkanDevice::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer * buffer, VkDeviceMemory * memory, void * data)
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

	VkResult allocResult = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, memory);

	VULKAN_CHECK_RESULT(allocResult);

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

	VkResult bindResult = vkBindBufferMemory(logicalDevice, *buffer, *memory, 0);

	VULKAN_CHECK_RESULT(bindResult);

	if (allocResult != VK_SUCCESS) {
		return allocResult;
	}
	else if (bindResult != VK_SUCCESS){
		return bindResult;
	}
	return VK_SUCCESS;
}

VkResult ng::vulkan::VulkanDevice::copyDataToBuffer(VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size, void * data)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkCommandBuffer commandBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, memoryCommandPool, true);

	createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		size,
		&stagingBuffer,
		&stagingBufferMemory,
		data
	);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = offset;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, dstBuffer, 1, &copyRegion);

	flushCommandBuffer(commandBuffer, memoryCommandPool, transferQueue, true);

	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);

}

uint32 ng::vulkan::VulkanDevice::getMemoryScore()
{
	uint32 score = 0;
	for (int i = 0; i < memoryProperties.memoryHeapCount; ++i) {
		LOGI("HeapIndex : " << i);
		LOGI("HeapSize : " << memoryProperties.memoryHeaps[i].size);
		LOGI("HeapFlags : " << memoryProperties.memoryHeaps[i].flags);

		for (int j = 0; j < memoryProperties.memoryTypeCount; ++j) {
			if (memoryProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
				//Memory Type 0
				if (memoryProperties.memoryTypes[j].heapIndex == i) {
					LOGI("Found VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT\n");
					score += memoryProperties.memoryHeaps[i].size / 10000;
				}
			}
			else if (memoryProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
				//Memory Type 1
				if (memoryProperties.memoryTypes[j].heapIndex == i) {
					LOGI("Found VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT\n");
					score += memoryProperties.memoryHeaps[i].size / 30000;
				}
			}
			else if (memoryProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
				//MemoryType 2
				if (memoryProperties.memoryTypes[j].heapIndex == i) {
					LOGI("Found VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT\n");
					score += memoryProperties.memoryHeaps[i].size / 50000;
				}
			}
			else if (memoryProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
				if (memoryProperties.memoryTypes[j].heapIndex == i) {
					LOGI("Found VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT\n");
					score += memoryProperties.memoryHeaps[i].size / 80000;
				}
			}
		}

	}
	return score;
}

uint32 ng::vulkan::VulkanDevice::getDeviceScore(
	VulkanDeviceTypeFlags deviceTypeFlags,
	std::vector<const char*> requiredExtentions,
	VkSurfaceKHR surface)
{
	uint32 score = 0;

	for (auto reqExtension : requiredExtentions) {
		if (!extensionSupported(reqExtension)) {
			return 0;
		}
	}


	std::pair<int32, int32> graphicsAndPresent;
	int32 transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
	if (transfer == -1) {
		return 0;
	}
	int32 compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

	if ((deviceTypeFlags & VULKAN_DEVICE_TYPE_DESCRETE_GRAPHICS_UNIT)) {
		if (features.geometryShader) {
			return 0;
		}
		if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return 0;
		}
		graphicsAndPresent = getGraphicsAndPresentQueueFamilyIndex(surface);
		if (graphicsAndPresent.first == -1) {
			return 0;
		}
		if (deviceTypeFlags & VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT) {

			if (graphicsAndPresent.second == -1) {
				return 0;
			}

			//Query swapchain support

			SwapchainSupportDetails details;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

			uint32 formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
			}

			uint32 presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
			}
			if (details.formats.empty() && details.presentModes.empty()) {
				return 0;
			}

			//end of swapchain querying

			if (!QueueFamilyIndices::isSame(graphicsAndPresent.first, graphicsAndPresent.second) && (deviceTypeFlags & VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT_IN_GRAPHICS_QUEUE)) {
				return 0;
			}
			else if(QueueFamilyIndices::isSame(graphicsAndPresent.first, graphicsAndPresent.second)){
				if (deviceTypeFlags & VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT_IN_GRAPHICS_QUEUE) {
					score += 2000;
				}
				else {
					score += 1000;
				}
			}

			if (!QueueFamilyIndices::isSame(graphicsAndPresent.first, transfer)) {
				score += 500;
			}
			if (!QueueFamilyIndices::isSame(graphicsAndPresent.second, transfer)) {
				score += 500;
			}
			score += (uint32)(properties.limits.maxImageDimension2D / 2);
		}
	}

	if (deviceTypeFlags & VULKAN_DEVICE_TYPE_DESCRETE_COMPUTE_UNIT) {
		if (compute == -1) {
			return 0;
		}

		if (!QueueFamilyIndices::isSame(compute, graphicsAndPresent.first)) {
			score += 1000;
		}
		if (!QueueFamilyIndices::isSame(compute, graphicsAndPresent.second)) {
			score += 1000;
		}
		if (!QueueFamilyIndices::isSame(compute, transfer)) {
			score += 1000;
		}

		if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return 0;
		}
	}

	score += getMemoryScore();
	return score;
}

