#include "debug.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>

ng::Debug::Debug()
{

}

ng::Debug::~Debug()
{

}

namespace ng {
	namespace graphics {
		namespace debug {

#ifdef NDEBUG
			const bool enableValidationLayers = false;
#else
			const bool enableValidationLayers = true;
#endif

			const std::vector<const char*> validationLayerNames = {
				"VK_LAYER_LUNARG_standard_validation"
			};

			int32_t validationLayerCount = 1;
			PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
			PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = VK_NULL_HANDLE;
			PFN_vkDebugReportMessageEXT debugBreakCallback = VK_NULL_HANDLE;

			VkDebugReportCallbackEXT msgCallback;
		}
	}
}

bool ng::graphics::debug::isValidationLayersEnabled()
{
	return enableValidationLayers;
}

const std::vector<const char*> ng::graphics::debug::getValidationLayerNames()
{
	return validationLayerNames;
}

bool ng::graphics::debug::checkValidationLayerSupport()
{
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	for (const char* layerName : validationLayerNames) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

std::vector<const char*> ng::graphics::debug::getRequiredExtensions(bool enableValidationLayers)
{
	std::vector<const char*> extensions;

	uint glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	for (uint i = 0; i < glfwExtensionCount; ++i) {
		extensions.push_back(glfwExtensions[i]);
	}
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
	return extensions;
}

VkBool32 ng::graphics::debug::messageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData)
{
	// Select prefix depending on flags passed to the callback
	// Note that multiple flags may be set for a single validation message
	std::string prefix("");

	// Error that may result in undefined behaviour
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		prefix += "ERROR:";
	};
	// Warnings may hint at unexpected / non-spec API usage
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		prefix += "WARNING:";
	};
	// May indicate sub-optimal usage of the API
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		prefix += "PERFORMANCE:";
	};
	// Informal messages that may become handy during debugging
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		prefix += "INFO:";
	}
	// Diagnostic info from the Vulkan loader and layers
	// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		prefix += "DEBUG:";
	}

	// Display message to default output (console/logcat)
	std::stringstream debugMessage;
	debugMessage << prefix << " [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		std::cerr << debugMessage.str() << "\n";
	}
	else {
		std::cout << debugMessage.str() << "\n";
	}
	fflush(stdout);

	// The return value of this callback controls wether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}

void ng::graphics::debug::setupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callback)
{
	ng::graphics::debug::CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	ng::graphics::debug::DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
	ng::graphics::debug::debugBreakCallback = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));

	VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)ng::graphics::debug::messageCallback;
	debugCreateInfo.flags = flags;

	VkResult err = ng::graphics::debug::CreateDebugReportCallback(
		instance,
		&debugCreateInfo,
		nullptr,
		(callback != VK_NULL_HANDLE) ? &callback : &ng::graphics::debug::msgCallback);
	assert(!err);
}

void ng::graphics::debug::freeDebugCallback(VkInstance instance)
{
	if (msgCallback != VK_NULL_HANDLE)
	{
		DestroyDebugReportCallback(instance, msgCallback, nullptr);
	}
}
