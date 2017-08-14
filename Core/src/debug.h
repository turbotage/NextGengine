#pragma once
#include "def.h"


namespace ng {
	namespace graphics {
		namespace debug {
			// Default debug callback
			VkBool32 messageCallback(VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objType,
				uint64_t srcObject,
				size_t location,
				int32_t msgCode,
				const char* pLayerPrefix,
				const char* pMsg,
				void* pUserData);

			// Load debug function pointers and set debug callback
			// if callBack is NULL, default message callback will be used
			void setupDebugging(
				VkInstance instance,
				VkDebugReportFlagsEXT flags,
				VkDebugReportCallbackEXT callBack);

			// Clear debug callback
			void freeDebugCallback(VkInstance instance);
		}
	}

	class Debug
	{
	public:
		Debug();
		~Debug();

	};
}

