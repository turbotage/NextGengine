#pragma once

#include "pch.h"

namespace ngv {

	class VulkanDevice;

	class VulkanThread {
	public:

		VulkanThread(VulkanDevice& device, uint32 nSwapChainImages);

		vk::CommandPool getCommandPool(uint32 swapChainIndex);

		vk::CommandBuffer getCommandBuffer(uint32 swapChainIndex);

	private:

		VulkanDevice& m_Device;

		std::vector<vk::CommandPool> m_CommandPools;
		std::vector<vk::CommandBuffer> m_CommandBuffers;


	};

}
