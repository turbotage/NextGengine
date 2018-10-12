#include "vulkan_base.h"
#include "vulkan_debug.h"

void ng::vulkan::VulkanBase::createInstance(std::vector<const char*> requiredExtensions)
{
	bool validationLayersEnabled = ng::vulkan::debug::isValidationLayersEnabled();
	if (validationLayersEnabled && !ng::vulkan::debug::checkValidationLayerSupport()) {
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

	if (validationLayersEnabled) {
		requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	createInfo.enabledExtensionCount= static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	if (validationLayersEnabled) {
		std::vector<const char*> validationLayers = ng::vulkan::debug::getDebugValidationLayers();
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	VULKAN_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
}

void ng::vulkan::VulkanBase::createDebugCallback()
{
	debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	ng::vulkan::debug::setupDebugging(instance, debugReportFlags, debugReportCallback);
}

ng::vulkan::VulkanBase::VulkanBase()
{

}

ng::vulkan::VulkanBase::~VulkanBase()
{
	debug::freeDebugCallback(instance, debugReportCallback);
	vkDestroyInstance(instance, nullptr);
}


