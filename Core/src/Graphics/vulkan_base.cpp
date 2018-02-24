#include "vulkan_base.h"
#include <iostream>
#include <map>
#include <utility>
#include <set>

const std::vector<const char*> graphicsDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> computeDeviceExtensions = {

};

void ng::graphics::VulkanBase::createInstance()
{
	bool validationLayersEnabled = ng::graphics::debug::isValidationLayersEnabled();
	if (validationLayersEnabled && !ng::graphics::debug::checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "NextGengine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	ng::graphics::debug::setDebugExtensions(this);
	createInfo. enabledExtensionCount= static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (validationLayersEnabled) {
		ng::graphics::debug::setDebugValidationLayers(this);
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	VULKAN_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
}

void ng::graphics::VulkanBase::freeInstance()
{
	vkDestroyInstance(instance, nullptr);
}

void ng::graphics::VulkanBase::createDebugCallback()
{
	debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debug::setupDebugging(instance, debugReportFlags, debugReportCallback);
}

void ng::graphics::VulkanBase::freeDebugCallback()
{
	debug::freeDebugCallback(instance, debugReportCallback);
}

void ng::graphics::VulkanBase::createPhysicalDevices()
{
	auto checkDeviceExtensionSupport = [&](VkPhysicalDevice device, short deviceType) -> bool {
		uint32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions;
		if (deviceType == GRAPHICS_UNIT) {
			std::set<std::string> requiredExtensions2(graphicsDeviceExtensions.begin(), graphicsDeviceExtensions.end());
			requiredExtensions = requiredExtensions2;
		}
		else if (deviceType == COMPUTE_UNIT) {
			std::set<std::string> requiredExtensions2(computeDeviceExtensions.begin(), computeDeviceExtensions.end());
			requiredExtensions = requiredExtensions2;
		}
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();

	};

	auto rateGraphicsDeviceSuitability = [&](VkPhysicalDevice device, bool needComputeCompatibility) -> uint32 {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		int score = 0;

		//things that is needed will return 0 if they don't exist
		//return 0 if no gemomatryShader is available
		if (!deviceFeatures.geometryShader) {
			return 0;
		}
		QueueFamilyIndices indices = findQueueFamilies(device);
		//if it doesn't support graphicsQueue return 0
		if (!indices.isGraphicsComplete()) {
			return 0;
		}
		//if it doesn't support presentQueue return 0
		if (!indices.isPresentComplete()) {
			return 0;
		}
		//if the graphics device also have to be the compute device return 0 if computeQueue isn't available
		if (needComputeCompatibility && !indices.isComputeComplete()) {
			return 0;
		}
		//check if graphicsDevice extensions are available, if not return 0
		if (!checkDeviceExtensionSupport(device, GRAPHICS_UNIT)) {
			return 0;
		}
		//if the graphicsDevice also needs to be computeDevice check if it also have computeDevice extensions
		//otherwise return 0
		if (needComputeCompatibility) {
			if (!checkDeviceExtensionSupport(device, COMPUTE_UNIT)) {
				return 0;
			}
		}
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		if (swapChainSupport.formats.empty() && swapChainSupport.presentModes.empty()) {
			return 0;
		}

		//score adding stuff
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}
		score += (int)(deviceProperties.limits.maxImageDimension2D / 2);
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
		printf("rate graphics device\n");
		printf("MemHeapCount : %d\n", memProperties.memoryHeapCount);
		printf("MemTypeCount : %d\n", memProperties.memoryTypeCount);

		for (int i = 0; i < memProperties.memoryHeapCount; ++i) {
			printf("HeapIndex : %d\n", i);
			printf("HeapSize : %" PRIu64 "\n", memProperties.memoryHeaps[i].size);
			printf("HeapFlags : %d\n", memProperties.memoryHeaps[i].flags);
			
			for (int j = 0; j < memProperties.memoryTypeCount; ++j) {
				if (memProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
					//Memory Type 0
					if (memProperties.memoryTypes[j].heapIndex == i) {
						printf("Found VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT\n");
						score += memProperties.memoryHeaps[i].size / 10000;
					}
				}
				else if (memProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
					//Memory Type 1
					if (memProperties.memoryTypes[j].heapIndex == i) {
						printf("Found VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT\n");
						score += memProperties.memoryHeaps[i].size / 30000;
					}
				}
				else if (memProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
					//MemoryType 2
					if (memProperties.memoryTypes[j].heapIndex == i) {
						printf("Found VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT\n");
						score += memProperties.memoryHeaps[i].size / 50000;
					}
				}
				else if (memProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
					if (memProperties.memoryTypes[j].heapIndex == i) {
						printf("Found VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT\n");
						score += memProperties.memoryHeaps[i].size / 80000;
					}
				}

			}
		}
		printf("physicalDevice score: %d\n", score);
		return score;
	};

	auto rateComputeDeviceSuitability = [&](VkPhysicalDevice device) -> uint32 {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		int score = 0;
		QueueFamilyIndices indices = findQueueFamilies(device);
		if (!indices.isComputeComplete()) {
			return 0;
		}
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
		printf("rate compute device\n");
		for (int i = 0; i < memProperties.memoryHeapCount; ++i) {
			
			printf("%" PRIu64 "\n", memProperties.memoryHeaps[i].size);
			printf("%d\n\n", memProperties.memoryHeaps[i].flags);
			printf("%d\n\n", memProperties.memoryTypes[i].propertyFlags);
			printf("%d\n\n", memProperties.memoryTypes[i].heapIndex);
			if (memProperties.memoryHeaps[i].flags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
				if (memProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
					score += (int)(memProperties.memoryHeaps[i].size / 1000000);
				}
				else if (memProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
					score += (int)(memProperties.memoryHeaps[i].size / 3000000);
				}
				else if (memProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
					score += (int)(memProperties.memoryHeaps[i].size / 5000000);
				}
				else {
					return 0;
				}
			}
			else {
				printf("found no VK_MEMORY_HEAP_DEVICE_LOCAL_BIT\n");
			}
			
		}
		printf("physicalDevice score: %d\n", score);
		return score;
	};

	uint32 deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	printf("Number of PhysicalDevices: %" PRIu32 "\n", deviceCount);

	auto pickGraphicsDevice = [&](std::vector<VkPhysicalDevice>* devicesIn, bool needComputeCapability = false) -> VkPhysicalDevice {
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : *devicesIn) {
			int score = rateGraphicsDeviceSuitability(device, needComputeCapability);
			candidates.insert(std::make_pair(score, device));
		}
		if (candidates.rbegin()->first > 0) {
			if (needComputeCapability) {
				computeAndGraphicsSameDevice = true;
			}
			return candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to find a suitable graphicsUnit");
		}
	};

	auto pickComputeDevice = [&](std::vector<VkPhysicalDevice>* devicesIn) -> VkPhysicalDevice {
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : *devicesIn) {
			int score = rateComputeDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}
		if (candidates.rbegin()->first > 0) {
			return candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to find a suitable computeUnit");
		}
	};

	//printf("DeviceCount: %d\n", deviceCount);
	if (!(deviceCount > 1)) {
		graphicsUnit.pDevice.device = pickGraphicsDevice(&devices, true);
		computeUnit.pDevice.device = graphicsUnit.pDevice.device;
	}
	else {
		graphicsUnit.pDevice.device = pickGraphicsDevice(&devices);
		devices.erase(std::find(devices.begin(), devices.end(), graphicsUnit.pDevice.device));
		computeUnit.pDevice.device = pickComputeDevice(&devices);
	}

	vkGetPhysicalDeviceMemoryProperties(graphicsUnit.pDevice.device , &graphicsUnit.pDevice.memoryProperties);
	vkGetPhysicalDeviceMemoryProperties(computeUnit.pDevice.device, &computeUnit.pDevice.memoryProperties);
}

