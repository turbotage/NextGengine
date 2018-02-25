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


