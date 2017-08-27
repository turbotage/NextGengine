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

			std::vector<const char*> validationLayers = {
				"VK_LAYER_LUNARG_standard_validation"
			};

			VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
				auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
				if (func != nullptr) {
					return func(instance, pCreateInfo, pAllocator, pCallback);
				}
				else {
					return VK_ERROR_EXTENSION_NOT_PRESENT;
				}
			}

			static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
				auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
				if (func != nullptr) {
					func(instance, callback, pAllocator);
				}
			}

		}
	}
}

bool ng::graphics::debug::isValidationLayersEnabled()
{
	return enableValidationLayers;
}

void ng::graphics::debug::setDebugValidationLayers(VulkanBase *vulkanBase)
{
	vulkanBase->layers = validationLayers;
}

bool ng::graphics::debug::checkValidationLayerSupport()
{
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	for (const char* layerName : validationLayers) {
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

void ng::graphics::debug::setDebugExtensions(VulkanBase *vulkanBase)
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
	vulkanBase->extensions = extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ng::graphics::debug::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64 obj, size_t location, int32 code, const char * layerPrefix, const char * msg, void * userData)
{
	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

void ng::graphics::debug::setupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callback)
{
	if (!enableValidationLayers) return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = flags;
	createInfo.pfnCallback = ng::graphics::debug::debugCallback;
	if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback");
	}
}

void ng::graphics::debug::freeDebugCallback(VkInstance instance, VkDebugReportCallbackEXT callback)
{
	DestroyDebugReportCallbackEXT(instance, callback, nullptr);
}
