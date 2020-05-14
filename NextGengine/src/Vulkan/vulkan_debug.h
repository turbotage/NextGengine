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

		extern std::list<std::string> validationLayerNames;

		struct VulkanMessage {
			vk::DebugReportFlagsEXT flags;
			vk::DebugReportObjectTypeEXT objType;
			uint64_t srcObject;
			size_t location;
			int32_t msgCode;
			const char* pLayerPrefix;
			const char* pMsg;
			void* pUserData;
		};

		using MessageHandler = std::function<VkBool32(const VulkanMessage& message)>;


		// Load debug function pointers and set debug callback
		// if callBack is NULL, default message callback will be used
		void setupDebugging(const vk::Instance& instance, const vk::DebugReportFlagsEXT& flags, const MessageHandler& handler = [](const VulkanMessage& m) {
			return VK_TRUE;
		});

		// Clear debug callback
		void freeDebugCallback(const vk::Instance& instance);


		namespace marker {
			extern bool active;

			void setup(const vk::Instance& instance, const vk::Device& device);

			void setObjectName(const vk::Device& device, uint64_t object, vk::DebugReportObjectTypeEXT objectType, const char* name);

			void setObjectTag(const vk::Device& device,
				uint64_t object, vk::DebugReportObjectTypeEXT objectType,
				uint64_t name,
				size_t tagSize,
				const void* tag);

			static std::array<float, 4> toFloatArray(const glm::vec4& color);

			void beginRegion(const vk::CommandBuffer& cmdBuffer, const std::string& markerName, const glm::vec4& color);

			void insert(const vk::CommandBuffer& cmdbuffer, const std::string& markerName, const glm::vec4& color);

			void endRegion(const vk::CommandBuffer& cmdbuffer);

			void setCommandBufferName(const vk::Device& device, const VkCommandBuffer& cmdBuffer, const char* name);

			void setQueueName(const vk::Device& device, const VkQueue& queue, const char* name);

			void setImageName(const vk::Device& device, const VkImage& image, const char* name);

			void setSamplerName(const vk::Device& device, const VkSampler& sampler, const char* name);

			void setBufferName(const vk::Device& device, const VkBuffer& buffer, const char* name);

			void setDeviceMemoryName(const vk::Device& device, const VkDeviceMemory& memory, const char* name);

			void setShaderModuleName(const vk::Device& device, const VkShaderModule& shaderModule, const char* name);

			void setPipelineName(const vk::Device& device, const VkPipeline& pipeline, const char* name);

			void setPipelineLayoutName(const vk::Device& device, const VkPipelineLayout& pipelineLayout, const char* name);

			void setRenderPassName(const vk::Device& device, const VkRenderPass& renderPass, const char* name);

			void setFramebufferName(const vk::Device& device, const VkFramebuffer& framebuffer, const char* name);

			void setDescriptorSetLayoutName(const vk::Device& device, const VkDescriptorSetLayout& descriptorSetLayout, const char* name);

			void setDescriptorSetName(const vk::Device& device, const VkDescriptorSet& descriptorSet, const char* name);

			void setSemaphoreName(const vk::Device& device, const VkSemaphore& semaphore, const char* name);

			void setFenceName(const vk::Device& device, const VkFence& fence, const char* name);

			void setEventName(const vk::Device& device, const VkEvent& _event, const char* name);

		}


	}
}