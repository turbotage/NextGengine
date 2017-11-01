#pragma once
#include "def.h"
#include <vector>
#include "Graphics\vulkan_base.h"

namespace ng {
	namespace graphics {
		namespace debug {
			
			bool isValidationLayersEnabled();

			bool checkValidationLayerSupport();
			
			void setDebugValidationLayers(VulkanBase *vulkanBase);

			void setDebugExtensions(VulkanBase *vulkanBase);

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
		}
	}

	class Debug
	{
	public:
		Debug();
		~Debug();

	};
}

