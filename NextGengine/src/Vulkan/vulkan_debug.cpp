#include "vulkan_debug.h"

using namespace ngv;

ngv::debug::VulkanDebugCallback::VulkanDebugCallback(vk::Instance instance, vk::DebugReportFlagsEXT flags)
	: m_Instance(instance)
{
	auto ci = vk::DebugReportCallbackCreateInfoEXT{ flags, &debugCallback };

	auto vkCreateDebugReportCallbackEXT =
		(PFN_vkCreateDebugReportCallbackEXT)m_Instance.getProcAddr("vkCreateDebugReportCallbackEXT");

	VkDebugReportCallbackEXT cb;
	vkCreateDebugReportCallbackEXT(m_Instance, 
		&(const VkDebugReportCallbackCreateInfoEXT&)ci,
		nullptr, &cb);

	m_Callback = cb;

}

void ngv::debug::VulkanDebugCallback::reset()
{
	if (m_Callback) {
		auto vkDestroyDebugReportCallbackEXT =
			(PFN_vkDestroyDebugReportCallbackEXT)m_Instance.getProcAddr(
				"vkDestroyDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackEXT(m_Instance, m_Callback, nullptr);
		m_Callback = vk::DebugReportCallbackEXT{};
	}
}



VKAPI_ATTR vk::Bool32 VKAPI_CALL ngv::debug::VulkanDebugCallback::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64 object, size_t location, int32 messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	printf("%08x debugCallback: %s\n\n", flags, pMessage);
	return VK_FALSE;
}
