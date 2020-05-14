#include "vulkan_context.h"

#include "vulkan_debug.h"

void ngv::VulkanDeviceCreateInfo::addQueueFamily(uint32 queueFamilyIndex, vk::ArrayProxy<float> priorities)
{
	deviceQueues.push_back({ {}, queueFamilyIndex });
	std::vector<float> prioritiesVector;
	prioritiesVector.resize(priorities.size());
	memcpy(prioritiesVector.data(), priorities.end(), 0.0f);
	deviceQueuesPriorities.push_back(prioritiesVector);
}

void ngv::VulkanDeviceCreateInfo::addQueueFamily(uint32 queueFamilyIndex, size_t count)
{
	std::vector<float> priorities;
	priorities.resize(count);
	std::fill(priorities.begin(), priorities.end(), 0.0f);
	addQueueFamily(queueFamilyIndex, priorities);
}

void ngv::VulkanDeviceCreateInfo::update()
{
	assert(deviceQueuesPriorities.size() == deviceQueues.size());
	auto size = deviceQueues.size();
	for (auto i = 0; i < size; ++i) {
		auto& deviceQueue = deviceQueues[i];
		auto& deviceQueuePriorities = deviceQueuesPriorities[i];
		deviceQueue.queueCount = static_cast<uint32>(deviceQueuePriorities.size());
		deviceQueue.pQueuePriorities = deviceQueuePriorities.data();
	}

	this->queueCreateInfoCount = static_cast<uint32>(deviceQueues.size());
	this->pQueueCreateInfos = deviceQueues.data();
}

//Context

// PUBLIC

std::set<std::string> ngv::VulkanContext::getAvailableLayers()
{
	std::set<std::string> result;
	auto layers = vk::enumerateInstanceLayerProperties();
	for (auto layer : layers) {
		result.insert(layer.layerName);
	}
	return result;
}

std::vector<vk::ExtensionProperties> ngv::VulkanContext::getExtensions()
{
	return vk::enumerateInstanceExtensionProperties();
}

std::set<std::string> ngv::VulkanContext::getExtensionNames()
{
	std::set<std::string> extensionNames;
	for (auto& ext : getExtensions()) {
		extensionNames.insert(ext.extensionName);
	}
	return extensionNames;
}

bool ngv::VulkanContext::isExtensionPresent(const std::string& extensionName)
{
	return getExtensionNames().count(extensionName) != 0;
}

std::vector<vk::ExtensionProperties> ngv::VulkanContext::getDeviceExtensions(const vk::PhysicalDevice& physicalDevice)
{
	return physicalDevice.enumerateDeviceExtensionProperties();
}

std::set<std::string> ngv::VulkanContext::getDeviceExtensionNames(const vk::PhysicalDevice& physicalDevice)
{
	std::set<std::string> extensionNames;
	for (auto& ext : getDeviceExtensions(physicalDevice)) {
		extensionNames.insert(ext.extensionName);
	}
	return extensionNames;
}

bool ngv::VulkanContext::isDeviceExtensionPresent(const vk::PhysicalDevice& physicalDevice, const std::string& extension)
{
	return getDeviceExtensionNames(physicalDevice).count(extension) != 0;
}

void ngv::VulkanContext::requireExtensions(const vk::ArrayProxy<const std::string>& requestedExtensions)
{
	m_RequiredExtensions.insert(requestedExtensions.begin(), requestedExtensions.end());
}

void ngv::VulkanContext::requireDeviceExtensions(const vk::ArrayProxy<const std::string>& requestedExtensions)
{
	m_RequiredDeviceExtensions.insert(requestedExtensions.begin(), requestedExtensions.end());
}

void ngv::VulkanContext::addInstanceExtensionPicker(const InstanceExtensionsPickerFunction& function)
{
	instanceExtensionsPickers.push_back(function);
}

void ngv::VulkanContext::setDevicePicker(const DevicePickerFunction& picker)
{
	devicePicker = picker;
}

void ngv::VulkanContext::setDeviceFeaturesPicker(const DeviceFeaturesPickerFunction& picker)
{
	deviceFeaturesPicker = picker;
}

void ngv::VulkanContext::setDeviceExtensionsPicker(const DeviceExtensionsPickerFunction& picker)
{
	deviceExtensionsPicker = picker;
}

void ngv::VulkanContext::setValidationEnabled(bool enable)
{
	if (instance != vk::Instance()) {
		throw std::runtime_error("Cannot change validations state after instance creation");
	}
	enableValidation = enable;
}

void ngv::VulkanContext::createInstance(uint32_t version)
{
	if (enableValidation) {
		requireExtensions({ (const char*)VK_EXT_DEBUG_REPORT_EXTENSION_NAME });
	}

	// Vulkan instance
	vk::ApplicationInfo appInfo;
	appInfo.pApplicationName = "VulkanExamples";
	appInfo.pEngineName = "VulkanExamples";
	appInfo.apiVersion = version;

	std::set<std::string> instanceExtensions;
	instanceExtensions.insert(m_RequiredExtensions.begin(), m_RequiredExtensions.end());
	for (const auto& picker : instanceExtensionsPickers) {
		auto extensions = picker();
		instanceExtensions.insert(extensions.begin(), extensions.end());
	}

	std::vector<const char*> enabledExtensions;
	for (const auto& extension : instanceExtensions) {
		enabledExtensions.push_back(extension.c_str());
	}

	// Enable surface extensions depending on os
	vk::InstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	if (!enabledExtensions.empty()) {
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}

	CStringVector layers;
	if (enableValidation) {
		layers = filterLayers(debug::validationLayerNames);
		instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
		instanceCreateInfo.ppEnabledLayerNames = layers.data();
	}

	instance = vk::createInstance(instanceCreateInfo);

	if (enableValidation) {
		debug::setupDebugging(instance, vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning);
	}

	dynamicDispatch.init(instance, &vkGetInstanceProcAddr);
}

void ngv::VulkanContext::createDevice(const vk::SurfaceKHR& surface)
{
	pickDevice(surface);
	buildDevice();
	dynamicDispatch.init(instance, &vkGetInstanceProcAddr, device, &vkGetDeviceProcAddr);


	if (enableDebugMarkers) {
		debug::marker::setup(instance, device);
	}

	pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());
	// Find a queue that supports graphics operations

	// Get the graphics queue
	queue = device.getQueue(queueIndices.graphics, 0);
}

void ngv::VulkanContext::destroy()
{
	if (queue) {
		queue.waitIdle();
	}
	device.waitIdle();
	for (const auto& trash : dumpster) {
		trash();
	}

	while (!recycler.empty()) {
		recycle();
	}

	destroyCommandPool();
	device.destroyPipelineCache(pipelineCache);
	device.destroy();
	if (enableValidation) {
		debug::freeDebugCallback(instance);
	}
	instance.destroy();
}

uint32_t ngv::VulkanContext::findQueue(const vk::QueueFlags& desiredFlags, const vk::SurfaceKHR& presentSurface) const
{
	uint32_t bestMatch{ VK_QUEUE_FAMILY_IGNORED };
	VkQueueFlags bestMatchExtraFlags{ VK_QUEUE_FLAG_BITS_MAX_ENUM };
	size_t queueCount = queueFamilyProperties.size();
	for (uint32_t i = 0; i < queueCount; ++i) {
		auto currentFlags = queueFamilyProperties[i].queueFlags;
		// Doesn't contain the required flags, skip it
		if (!(currentFlags & desiredFlags)) {
			continue;
		}

		if (presentSurface && VK_FALSE == physicalDevice.getSurfaceSupportKHR(i, presentSurface)) {
			continue;
		}
		VkQueueFlags currentExtraFlags = (currentFlags & ~desiredFlags).operator VkQueueFlags();

		// If we find an exact match, return immediately
		if (0 == currentExtraFlags) {
			return i;
		}

		if (bestMatch == VK_QUEUE_FAMILY_IGNORED || currentExtraFlags < bestMatchExtraFlags) {
			bestMatch = i;
			bestMatchExtraFlags = currentExtraFlags;
		}
	}

	return bestMatch;
}

void ngv::VulkanContext::trashPipeline(vk::Pipeline& pipeline) const
{
	trash<vk::Pipeline>(pipeline, [this](vk::Pipeline pipeline) { device.destroyPipeline(pipeline); });
}

void ngv::VulkanContext::trashCommandBuffers(const vk::CommandPool& commandPool, std::vector<vk::CommandBuffer>& cmdBuffers) const
{
	std::function<void(const std::vector<vk::CommandBuffer>& t)> destructor = [=](const std::vector<vk::CommandBuffer>& cmdBuffers) {
		device.freeCommandBuffers(commandPool, cmdBuffers);
	};
	trashAll(cmdBuffers, destructor);
}

void ngv::VulkanContext::emptyDumpster(vk::Fence fence)
{
	VoidLambdaList newDumpster;
	newDumpster.swap(dumpster);
	recycler.push(FencedLambda{ fence, [newDumpster] {
								   for (const auto& f : newDumpster) {
									   f();
								   }
							   } });
}

void ngv::VulkanContext::recycle()
{
	while (!recycler.empty() && vk::Result::eSuccess == device.getFenceStatus(recycler.front().first)) {
		vk::Fence fence = recycler.front().first;
		VoidLambda lambda = recycler.front().second;
		recycler.pop();

		lambda();

		if (recycler.empty() || fence != recycler.front().first) {
			device.destroyFence(fence);
		}
	}
}

void ngv::VulkanContext::setImageLayout(vk::CommandBuffer cmdbuffer, vk::Image image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange) const
{
	// Create an image barrier object
	vk::ImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.srcAccessMask = vks::util::accessFlagsForLayout(oldImageLayout);
	imageMemoryBarrier.dstAccessMask = vks::util::accessFlagsForLayout(newImageLayout);
	vk::PipelineStageFlags srcStageMask = vks::util::pipelineStageForLayout(oldImageLayout);
	vk::PipelineStageFlags destStageMask = vks::util::pipelineStageForLayout(newImageLayout);
	// Put barrier on top
	// Put barrier inside setup command buffer
	cmdbuffer.pipelineBarrier(srcStageMask, destStageMask, vk::DependencyFlags(), nullptr, nullptr, imageMemoryBarrier);
}

void ngv::VulkanContext::setImageLayout(vk::CommandBuffer cmdbuffer, vk::Image image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout) const
{
	setImageLayout(cmdbuffer, image, vk::ImageAspectFlagBits::eColor, oldImageLayout, newImageLayout);
}

void ngv::VulkanContext::setImageLayout(vk::CommandBuffer cmdbuffer, vk::Image image, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout) const
{
	vk::ImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = aspectMask;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange);
}

void ngv::VulkanContext::setImageLayout(vk::Image image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange) const
{
	withPrimaryCommandBuffer([&](const auto& commandBuffer) { setImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange); });
}

void ngv::VulkanContext::setImageLayout(vk::Image image, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout) const
{
	withPrimaryCommandBuffer([&](const auto& commandBuffer) { setImageLayout(commandBuffer, image, aspectMask, oldImageLayout, newImageLayout); });
}

vk::CommandPool ngv::VulkanContext::getCommandPool() const
{
	if (!m_CmdPool) {
		vk::CommandPoolCreateInfo cmdPoolInfo;
		cmdPoolInfo.queueFamilyIndex = queueIndices.graphics;
		cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		m_CmdPool = device.createCommandPool(cmdPoolInfo);
	}
	return m_CmdPool;
}

void ngv::VulkanContext::destroyCommandPool() const
{
	if (m_CmdPool) {
		device.destroyCommandPool(m_CmdPool);
		m_CmdPool = vk::CommandPool();
	}
}

std::vector<vk::CommandBuffer> ngv::VulkanContext::allocateCommandBuffers(uint32_t count, vk::CommandBufferLevel level) const
{
	std::vector<vk::CommandBuffer> result;
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.commandPool = getCommandPool();
	commandBufferAllocateInfo.commandBufferCount = count;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	result = device.allocateCommandBuffers(commandBufferAllocateInfo);
	return result;
}

vk::CommandBuffer ngv::VulkanContext::createCommandBuffer(vk::CommandBufferLevel level) const
{
	vk::CommandBuffer cmdBuffer;
	vk::CommandBufferAllocateInfo cmdBufAllocateInfo;
	cmdBufAllocateInfo.commandPool = getCommandPool();
	cmdBufAllocateInfo.level = level;
	cmdBufAllocateInfo.commandBufferCount = 1;
	cmdBuffer = device.allocateCommandBuffers(cmdBufAllocateInfo)[0];
	return cmdBuffer;
}

void ngv::VulkanContext::flushCommandBuffer(vk::CommandBuffer& commandBuffer) const
{
	if (!commandBuffer) {
		return;
	}
	queue.submit(vk::SubmitInfo{ 0, nullptr, nullptr, 1, &commandBuffer }, vk::Fence());
	queue.waitIdle();
	device.waitIdle();
}

void ngv::VulkanContext::withPrimaryCommandBuffer(const std::function<void(const vk::CommandBuffer& commandBuffer)>& f) const
{
	vk::CommandBuffer commandBuffer = createCommandBuffer(vk::CommandBufferLevel::ePrimary);
	commandBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	f(commandBuffer);
	commandBuffer.end();
	flushCommandBuffer(commandBuffer);
	device.freeCommandBuffers(getCommandPool(), commandBuffer);
}

void ngv::VulkanContext::copyToMemory(const vk::DeviceMemory& memory, const void* data, vk::DeviceSize size, vk::DeviceSize offset) const
{
	void* mapped = device.mapMemory(memory, offset, size, vk::MemoryMapFlags());
	memcpy(mapped, data, size);
	device.unmapMemory(memory);
}

vk::Bool32 ngv::VulkanContext::getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties, uint32_t* typeIndex) const
{
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				*typeIndex = i;
				return VK_TRUE;
			}
		}
		typeBits >>= 1;
	}
	return VK_FALSE;
}

uint32_t ngv::VulkanContext::getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties) const
{
	uint32_t result = 0;
	if (VK_FALSE == getMemoryType(typeBits, properties, &result)) {
		throw std::runtime_error("Unable to find memory type " + vk::to_string(properties));
		// todo : throw error
	}
	return result;
}

void ngv::VulkanContext::submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers, const vk::ArrayProxy<const vk::Semaphore>& wait, const vk::ArrayProxy<const vk::PipelineStageFlags>& waitStages, const vk::ArrayProxy<const vk::Semaphore>& signals, const vk::Fence& fence) const
{
	vk::SubmitInfo info;
	info.commandBufferCount = commandBuffers.size();
	info.pCommandBuffers = commandBuffers.data();

	if (!signals.empty()) {
		info.signalSemaphoreCount = signals.size();
		info.pSignalSemaphores = signals.data();
	}

	assert(waitStages.size() == wait.size());

	if (!wait.empty()) {
		info.waitSemaphoreCount = wait.size();
		info.pWaitSemaphores = wait.data();
		info.pWaitDstStageMask = waitStages.data();
	}
	info.pWaitDstStageMask = waitStages.data();

	info.signalSemaphoreCount = signals.size();
	queue.submit(info, fence);
}

void ngv::VulkanContext::submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers, const vk::ArrayProxy<const SemaphoreStagePair>& wait, const vk::ArrayProxy<const vk::Semaphore>& signals, const vk::Fence& fence) const
{
	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStages;
	for (size_t i = 0; i < wait.size(); ++i) {
		const auto& pair = wait.data()[i];
		waitSemaphores.push_back(pair.first);
		waitStages.push_back(pair.second);
	}
	submit(commandBuffers, waitSemaphores, waitStages, signals, fence);
}

void ngv::VulkanContext::submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers, const SemaphoreStagePair& wait, const vk::ArrayProxy<const vk::Semaphore>& signals, const vk::Fence& fence) const
{
	submit(commandBuffers, wait.first, wait.second, signals, fence);
}

vk::Format ngv::VulkanContext::getSupportedDepthFormat() const
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use
		// Start with the highest precision packed format
	std::vector<vk::Format> depthFormats = { vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint,
											 vk::Format::eD16Unorm };

	for (auto& format : depthFormats) {
		vk::FormatProperties formatProps;
		formatProps = physicalDevice.getFormatProperties(format);
		// vk::Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
			return format;
		}
	}

	throw std::runtime_error("No supported depth format");
}




// PROTECTED

void ngv::VulkanContext::pickDevice(const vk::SurfaceKHR& surface)
{
	// Physical device
	physicalDevices = instance.enumeratePhysicalDevices();

	// Note :
	// This example will always use the first physical device reported,
	// change the vector index if you have multiple Vulkan devices installed
	// and want to use another one
	physicalDevice = devicePicker(physicalDevices);
	struct Version {
		uint32_t patch : 12;
		uint32_t minor : 10;
		uint32_t major : 10;
	} _version;
	// Store properties (including limits) and features of the phyiscal device
	// So examples can check against them and see if a feature is actually supported
	queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	deviceProperties = physicalDevice.getProperties();
	memcpy(&_version, &deviceProperties.apiVersion, sizeof(uint32_t));
	deviceFeatures = physicalDevice.getFeatures();
	// Gather physical device memory properties
	deviceMemoryProperties = physicalDevice.getMemoryProperties();
	queueIndices.graphics = findQueue(vk::QueueFlagBits::eGraphics, surface);
	queueIndices.compute = findQueue(vk::QueueFlagBits::eCompute);
	queueIndices.transfer = findQueue(vk::QueueFlagBits::eTransfer);
}

void ngv::VulkanContext::buildDevice()
{
	// Vulkan device
	vks::queues::DeviceCreateInfo deviceCreateInfo;

	deviceFeaturesPicker(physicalDevice, enabledFeatures2);
	if (enabledFeatures2.pNext) {
		deviceCreateInfo.pNext = &enabledFeatures2;
	}
	else {
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
	}

	deviceCreateInfo.addQueueFamily(queueIndices.graphics, queueFamilyProperties[queueIndices.graphics].queueCount);
	if (queueIndices.compute != VK_QUEUE_FAMILY_IGNORED && queueIndices.compute != queueIndices.graphics) {
		deviceCreateInfo.addQueueFamily(queueIndices.compute, queueFamilyProperties[queueIndices.compute].queueCount);
	}
	if (queueIndices.transfer != VK_QUEUE_FAMILY_IGNORED && queueIndices.transfer != queueIndices.graphics &&
		queueIndices.transfer != queueIndices.compute) {
		deviceCreateInfo.addQueueFamily(queueIndices.transfer, queueFamilyProperties[queueIndices.transfer].queueCount);
	}
	deviceCreateInfo.update();

	std::set<std::string> allDeviceExtensions = deviceExtensionsPicker(physicalDevice);
	allDeviceExtensions.insert(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

	std::vector<const char*> enabledExtensions;
	for (const auto& extension : allDeviceExtensions) {
		enabledExtensions.push_back(extension.c_str());
	}

	// enable the debug marker extension if it is present (likely meaning a debugging tool is present)
	if (isDeviceExtensionPresent(physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
		enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		enableDebugMarkers = true;
	}

	if (!enabledExtensions.empty()) {
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	device = physicalDevice.createDevice(deviceCreateInfo);
}

// PRIVATE

ngv::CStringVector ngv::VulkanContext::toCStrings(const StringList& values)
{
	CStringVector result;
	result.reserve(values.size());
	for (const auto& string : values) {
		result.push_back(string.c_str());
	}
	return result;
}

ngv::CStringVector ngv::VulkanContext::toCStrings(const vk::ArrayProxy<const std::string>& values)
{
	CStringVector result;
	result.reserve(values.size());
	for (const auto& string : values) {
		result.push_back(string.c_str());
	}
	return result;
}

ngv::CStringVector ngv::VulkanContext::filterLayers(const StringList& desiredLayers)
{
	static std::set<std::string> validLayerNames = getAvailableLayers();
	CStringVector result;
	for (const auto& string : desiredLayers) {
		if (validLayerNames.count(string) != 0) {
			result.push_back(string.c_str());
		}
	}
	return result;
}



