#pragma once

#include "vulkandef.h"

#include <algorithm>
#include <functional>
#include <list>
#include <string>
#include <mutex>
#include <sstream>
#include <iostream>

#include <glm/vec4.hpp>

namespace ngv {
	namespace debug {

		class VulkanDebugCallback {
		public:

			VulkanDebugCallback() {}

			VulkanDebugCallback(vk::Instance instance, vk::DebugReportFlagsEXT flags =
				vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError);
				

			~VulkanDebugCallback() {};

			void reset();

		private:

			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objectType, uint64 object, size_t location, int32 messageCode,
				const char* pLayerPrefix, const char* pMessage, void* pUserData);

		private:
			vk::DebugReportCallbackEXT m_Callback;
			vk::Instance m_Instance;

		};

	}
}