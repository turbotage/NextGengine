#include "debug.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>

#include "Graphics/vulkan_base.h"

ng::debug::Debug::Debug()
{
}

ng::debug::Debug::~Debug()
{
}

void ng::debug::exitFatal(std::string message, int32 exitCode)
{
	LOGI("Fatal error : %s", message.c_str());
	exit(exitCode);
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

std::vector<const char*> ng::graphics::debug::getDebugValidationLayers()
{
	return ng::graphics::debug::validationLayers;
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

std::string ng::graphics::debug::errorString(VkResult errorCode)
{
	switch (errorCode) {
#define STR(r) case ##r: return #r
		STR(VK_NOT_READY);
		STR(VK_TIMEOUT);
		STR(VK_EVENT_SET);
		STR(VK_EVENT_RESET);
		STR(VK_INCOMPLETE);
		STR(VK_ERROR_OUT_OF_HOST_MEMORY);
		STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
		STR(VK_ERROR_INITIALIZATION_FAILED);
		STR(VK_ERROR_DEVICE_LOST);
		STR(VK_ERROR_MEMORY_MAP_FAILED);
		STR(VK_ERROR_LAYER_NOT_PRESENT);
		STR(VK_ERROR_EXTENSION_NOT_PRESENT);
		STR(VK_ERROR_FEATURE_NOT_PRESENT);
		STR(VK_ERROR_INCOMPATIBLE_DRIVER);
		STR(VK_ERROR_TOO_MANY_OBJECTS);
		STR(VK_ERROR_FORMAT_NOT_SUPPORTED);
		STR(VK_ERROR_SURFACE_LOST_KHR);
		STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(VK_SUBOPTIMAL_KHR);
		STR(VK_ERROR_OUT_OF_DATE_KHR);
		STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(VK_ERROR_VALIDATION_FAILED_EXT);
		STR(VK_ERROR_INVALID_SHADER_NV);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}

void ng::graphics::debug::exitFatal(std::string message, VkResult result)
{
	ng::debug::exitFatal(message, (int32)result);
}
