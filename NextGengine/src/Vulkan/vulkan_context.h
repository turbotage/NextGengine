#pragma once

#include "vulkandef.h"
#include "vulkan_debug.h"

#include <set>
#include <functional>
#include <list>
#include <queue>


namespace ngv {

	class VulkanContext {
	public:

		VulkanContext();

		VulkanContext(const std::string& name);

		~VulkanContext();

		vk::Instance getInstance();

		vk::Device getDevice();

	private:

		vk::UniqueInstance m_Instance;
		ngv::debug::VulkanDebugCallback m_DebugCallback;
		vk::UniqueDevice m_Device;

		vk::PhysicalDevice m_PhysicalDevice;

		vk::UniquePipelineCache m_PipelineCache;
		
		std::vector<vk::UniqueDescriptorPool> m_DescriptorPools;

		uint32 m_GraphicsQueueFamilyIndex;
		uint32 m_ComputeQueueFamilyIndex;

		vk::PhysicalDeviceMemoryProperties m_MemProps;

		bool m_Ok = false;
	};

}