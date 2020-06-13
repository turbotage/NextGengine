#include "vulkan_thread.h"

ngv::VulkanThread::VulkanThread(VulkanDevice& device, uint32 nSwapChainImages)
	: m_Device(device)
{
	m_CommandPools.resize(nSwapChainImages);
	m_CommandBuffers.resize(nSwapChainImages);
}

vk::CommandPool ngv::VulkanThread::getCommandPool(uint32 swapChainIndex)
{
	return m_CommandPools[swapChainIndex];
}

vk::CommandBuffer ngv::VulkanThread::getCommandBuffer(uint32 swapChainIndex)
{
	return m_CommandBuffers[swapChainIndex];
}
