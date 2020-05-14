#include "vulkan_debug.h"

using namespace ngv;

namespace ngv {
	namespace debug {

		std::list<std::string> validationLayerNames = {
			// This is a meta layer that enables all of the standard
			// validation layers in the correct order :
			// threading, parameter_validation, device_limits, object_tracker, image, core_validation, swapchain, and unique_objects
			"VK_LAYER_LUNARG_assistant_layer", "VK_LAYER_LUNARG_standard_validation"
		};

		static std::once_flag dispatcherInitFlag;
		vk::DispatchLoaderDynamic dispatcher;
		vk::DebugReportCallbackEXT msgCallback;

		VkBool32 messageCallback(VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t srcObject,
			size_t location,
			int32_t msgCode,
			const char* pLayerPrefix,
			const char* pMsg,
			void* pUserData) {
			std::string message;
			{
				std::stringstream buf;
				if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
					buf << "ERROR: ";
				}
				else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
					buf << "WARNING: ";
				}
				else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
					buf << "PERF: ";
				}
				else {
					return false;
				}
				buf << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;
				message = buf.str();
			}

			std::cout << message << std::endl;

#ifdef _MSC_VER
			OutputDebugStringA(message.c_str());
			OutputDebugStringA("\n");
#endif
			return false;
		}


		namespace marker {
			bool active = false;
			static std::once_flag markerDispatcherInitFlag;
			vk::DispatchLoaderDynamic markerDispatcher;
		}


	}
}

void ngv::debug::setupDebugging(const vk::Instance& instance, const vk::DebugReportFlagsEXT& flags, const MessageHandler& handler)
{
	std::call_once(dispatcherInitFlag, [&] { dispatcher.init(instance, &vkGetInstanceProcAddr); });
	vk::DebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
	dbgCreateInfo.flags = flags;
	msgCallback = instance.createDebugReportCallbackEXT(dbgCreateInfo, nullptr, dispatcher);
}

void ngv::debug::freeDebugCallback(const vk::Instance& instance)
{
	std::call_once(dispatcherInitFlag, [&] { dispatcher.init(instance, &vkGetInstanceProcAddr); });
	instance.destroyDebugReportCallbackEXT(msgCallback, nullptr, dispatcher);
}

void ngv::debug::marker::setup(const vk::Instance& instance, const vk::Device& device)
{
	std::call_once(markerDispatcherInitFlag, [&] { markerDispatcher.init(instance, &vkGetInstanceProcAddr, device, &vkGetDeviceProcAddr); });
	// Set flag if at least one function pointer is present
	active = (markerDispatcher.vkDebugMarkerSetObjectNameEXT != VK_NULL_HANDLE);
}

void ngv::debug::marker::setObjectName(const vk::Device& device, uint64_t object, vk::DebugReportObjectTypeEXT objectType, const char* name)
{
	// Check for valid function pointer (may not be present if not running in a debugging application)
	if (markerDispatcher.vkDebugMarkerSetObjectNameEXT) {
		device.debugMarkerSetObjectNameEXT({ objectType, object, name }, markerDispatcher);
	}
}

void ngv::debug::marker::setObjectTag(const vk::Device& device, uint64_t object, vk::DebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag)
{
	// Check for valid function pointer (may not be present if not running in a debugging application)
	if (markerDispatcher.vkDebugMarkerSetObjectTagEXT) {
		device.debugMarkerSetObjectTagEXT({ objectType, object, name, tagSize, tag }, markerDispatcher);
	}
}

std::array<float, 4> ngv::debug::marker::toFloatArray(const glm::vec4& color)
{
	return { color.r, color.g, color.b, color.a };
}

void ngv::debug::marker::beginRegion(const vk::CommandBuffer& cmdBuffer, const std::string& markerName, const glm::vec4& color)
{
	// Check for valid function pointer (may not be present if not running in a debugging application)
	if (markerDispatcher.vkCmdDebugMarkerBeginEXT) {
		cmdBuffer.debugMarkerBeginEXT({ markerName.c_str(), toFloatArray(color) }, markerDispatcher);
	}
}

void ngv::debug::marker::insert(const vk::CommandBuffer& cmdbuffer, const std::string& markerName, const glm::vec4& color)
{
	// Check for valid function pointer (may not be present if not running in a debugging application)
	if (markerDispatcher.vkCmdDebugMarkerInsertEXT) {
		cmdbuffer.debugMarkerInsertEXT({ markerName.c_str(), toFloatArray(color) }, markerDispatcher);
	}
}

void ngv::debug::marker::endRegion(const vk::CommandBuffer& cmdbuffer)
{
	// Check for valid function (may not be present if not runnin in a debugging application)
	if (markerDispatcher.vkCmdDebugMarkerEndEXT) {
		cmdbuffer.debugMarkerEndEXT(markerDispatcher);
	}
}

void ngv::debug::marker::setCommandBufferName(const vk::Device& device, const VkCommandBuffer& cmdBuffer, const char* name)
{
	setObjectName(device, (uint64_t)cmdBuffer, vk::DebugReportObjectTypeEXT::eCommandBuffer, name);
}

void ngv::debug::marker::setQueueName(const vk::Device& device, const VkQueue& queue, const char* name)
{
	setObjectName(device, (uint64_t)queue, vk::DebugReportObjectTypeEXT::eQueue, name);
}

void ngv::debug::marker::setImageName(const vk::Device& device, const VkImage& image, const char* name)
{
	setObjectName(device, (uint64_t)image, vk::DebugReportObjectTypeEXT::eImage, name);
}

void ngv::debug::marker::setSamplerName(const vk::Device& device, const VkSampler& sampler, const char* name)
{
	setObjectName(device, (uint64_t)sampler, vk::DebugReportObjectTypeEXT::eSampler, name);
}

void ngv::debug::marker::setBufferName(const vk::Device& device, const VkBuffer& buffer, const char* name)
{
	setObjectName(device, (uint64_t)buffer, vk::DebugReportObjectTypeEXT::eBuffer, name);
}

void ngv::debug::marker::setDeviceMemoryName(const vk::Device& device, const VkDeviceMemory& memory, const char* name)
{
	setObjectName(device, (uint64_t)memory, vk::DebugReportObjectTypeEXT::eDeviceMemory, name);
}

void ngv::debug::marker::setShaderModuleName(const vk::Device& device, const VkShaderModule& shaderModule, const char* name)
{
	setObjectName(device, (uint64_t)shaderModule, vk::DebugReportObjectTypeEXT::eShaderModule, name);
}

void ngv::debug::marker::setPipelineName(const vk::Device& device, const VkPipeline& pipeline, const char* name)
{
	setObjectName(device, (uint64_t)pipeline, vk::DebugReportObjectTypeEXT::ePipeline, name);
}

void ngv::debug::marker::setPipelineLayoutName(const vk::Device& device, const VkPipelineLayout& pipelineLayout, const char* name)
{
	setObjectName(device, (uint64_t)pipelineLayout, vk::DebugReportObjectTypeEXT::ePipelineLayout, name);
}

void ngv::debug::marker::setRenderPassName(const vk::Device& device, const VkRenderPass& renderPass, const char* name)
{
	setObjectName(device, (uint64_t)renderPass, vk::DebugReportObjectTypeEXT::eRenderPass, name);
}

void ngv::debug::marker::setFramebufferName(const vk::Device& device, const VkFramebuffer& framebuffer, const char* name)
{
	setObjectName(device, (uint64_t)framebuffer, vk::DebugReportObjectTypeEXT::eFramebuffer, name);
}

void ngv::debug::marker::setDescriptorSetLayoutName(const vk::Device& device, const VkDescriptorSetLayout& descriptorSetLayout, const char* name)
{
	setObjectName(device, (uint64_t)descriptorSetLayout, vk::DebugReportObjectTypeEXT::eDescriptorSetLayout, name);
}

void ngv::debug::marker::setDescriptorSetName(const vk::Device& device, const VkDescriptorSet& descriptorSet, const char* name)
{
	setObjectName(device, (uint64_t)descriptorSet, vk::DebugReportObjectTypeEXT::eDescriptorSet, name);
}

void ngv::debug::marker::setSemaphoreName(const vk::Device& device, const VkSemaphore& semaphore, const char* name)
{
	setObjectName(device, (uint64_t)semaphore, vk::DebugReportObjectTypeEXT::eSemaphore, name);
}

void ngv::debug::marker::setFenceName(const vk::Device& device, const VkFence& fence, const char* name)
{
	setObjectName(device, (uint64_t)fence, vk::DebugReportObjectTypeEXT::eFence, name);
}

void ngv::debug::marker::setEventName(const vk::Device& device, const VkEvent& _event, const char* name)
{
	setObjectName(device, (uint64_t)_event, vk::DebugReportObjectTypeEXT::eEvent, name);
}
