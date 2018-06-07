#pragma once

#include "../def.h"

namespace ng {
	namespace vulkan {

		class VulkanBase;

		namespace debug {

			bool isValidationLayersEnabled();

			bool checkValidationLayerSupport();

			std::vector<const char*> getDebugValidationLayers();

			VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
				VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objType,
				uint64 obj,
				size_t location,
				int32 code,
				const char* layerPrefix,
				const char* msg,
				void* userData);

			// Load debug function pointers and set debug callback
			// if callBack is NULL, default message callback will be used
			void setupDebugging(
				VkInstance instance,
				VkDebugReportFlagsEXT flags,
				VkDebugReportCallbackEXT callback);

			// Clear debug callback
			void freeDebugCallback(VkInstance instance, VkDebugReportCallbackEXT callback);

			std::string errorString(VkResult errorCode);

			void exitFatal(std::string message, VkResult result);
		}
	}
}
