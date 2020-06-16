#pragma once

#include "vulkandef.h"
#include "vulkan_debug.h"

#include <set>
#include <functional>
#include <list>
#include <queue>


namespace ngv {

	class VulkanDevice;

	class VulkanContext {
	public:

		VulkanContext();

		VulkanContext(const std::string& name, uint32 version = NG_MAKE_VERSION(1,0,0));

		void dumpCaps(std::ostream& os) const;
		/// Get the Vulkan instance.
		const vk::Instance instance() const;
		/// Get the Vulkan device.
		const vk::Device device() const;
		/// Get the queue used to submit graphics jobs
		const vk::Queue graphicsQueue() const;
		/// Get the queue used to submit compute jobs
		const vk::Queue computeQueue() const;
		/// Get the vulkanDevice
		VulkanDevice& vulkanDevice() const;
		/// Get the physical device.
		const vk::PhysicalDevice& physicalDevice() const;
		/// Get the default pipeline cache (you can use your own if you like).
		const vk::PipelineCache pipelineCache() const;

		//TODO: some sort of get descriptorPool

		/// Get the family index for the graphics queues.
		uint32_t graphicsQueueFamilyIndex() const;
		/// Get the family index for the compute queues.
		uint32 computeQueueFamilyIndex() const;

		const vk::PhysicalDeviceMemoryProperties& memProps() const;

		~VulkanContext();

		VulkanContext& operator=(VulkanContext&& context) = default;

		bool ok() const;

	private:

		vk::UniqueInstance m_Instance;
		ngv::debug::VulkanDebugCallback m_DebugCallback;
		
		std::unique_ptr<VulkanDevice> m_pDevice;

		vk::UniquePipelineCache m_PipelineCache;
		
		std::vector<vk::UniqueDescriptorPool> m_DescriptorPools;

		uint32 m_GraphicsQueueFamilyIndex = 0;
		uint32 m_ComputeQueueFamilyIndex = 0;

		bool m_Ok = false;
	};

}