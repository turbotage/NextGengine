#include "vulkan_thread_pool.h"
#include "../vulkan_device.h"

ng::vulkan::VulkanThreadPool::VulkanThreadPool(const int nWorkThreads, ng::vulkan::VulkanDevice* vulkanDevice, bool workAndTrasferSamePool = false)
{
	if (nWorkThreads < 1) {
		throw std::runtime_error("tried to create vulkan thread-pool with less than one thread");
	}

	m_VulkanDevice = vulkanDevice;

	for (int i = 1; i < nWorkThreads + 1; ++i) {
		m_WorkPool.emplace_back(i);

		m_WorkPool[i].commandPool = vulkanDevice->createCommandPool(vulkanDevice->queueFamilyIndices.graphics);
	}
}

ng::vulkan::VulkanThreadPool::~VulkanThreadPool()
{
	if (m_WorkPool.size() > 0) {
		shutdown();
	}
}

void ng::vulkan::VulkanThreadPool::shutdown()
{

	for (auto& worker : m_WorkPool) {
		worker.stop();
	}

	while (m_WorkPool.size() > 0) {
		for (int i = 0; i < m_WorkPool.size(); ++i) {
			if (m_WorkPool[i].thread.joinable()) {
				m_WorkPool[i].thread.join();
				m_WorkPool.erase(m_WorkPool.begin()+i);
			}
		}
		std::this_thread::yield();
	}
}

VkCommandBuffer ng::vulkan::VulkanThreadPool::getSecondaryCommandBuffer(uint16 i)
{

}
