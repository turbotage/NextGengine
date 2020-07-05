#pragma once

#include "pch.h"

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