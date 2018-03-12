#pragma once
#include "def.h"

namespace ng {



	namespace debug {

#ifdef NDEBUG
#define LOGD(s)
#else
#define LOGD(s)															\
{																		\
	std::cout << "LOGD: " << s << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
}
#endif

#define USE_LOGI

#ifdef USE_LOGI
#define LOGI(s)															\
{																		\
	std::cout << "LOGI: " << s << std::endl;							\
}
#else
#define LOGI(s)
#endif

		class Debug
		{
		public:
			Debug();
			~Debug();

		};

		void exitFatal(std::string message, int32 exitCode);

	}

	namespace graphics {

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