#include "vulkan_device.h"



// <============================ DEVICE MAKER ==============================>
ngv::VulkanDeviceMaker& ngv::VulkanDeviceMaker::setDefaultLayers()
{
    m_Layers.push_back("VK_LAYER_LUNARG_standard_validation");
    m_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    return *this;
}

ngv::VulkanDeviceMaker& ngv::VulkanDeviceMaker::addLayer(const char* layer)
{
    m_Layers.push_back(layer);
    return *this;
}

ngv::VulkanDeviceMaker& ngv::VulkanDeviceMaker::addExtension(const char* layer)
{
    m_DeviceExtensions.push_back(layer);
    return *this;
}

ngv::VulkanDeviceMaker& ngv::VulkanDeviceMaker::addQueue(uint32 familyIndex, float priority, uint32 n)
{
    m_QueuePriorities.emplace_back(n, priority);

    m_QueueCreateInfos.emplace_back(
        vk::DeviceQueueCreateFlags{},
        familyIndex, n,
        m_QueuePriorities.back().data()
    );

    return *this;
}

vk::UniqueDevice ngv::VulkanDeviceMaker::createUnique(vk::PhysicalDevice physicalDevice)
{
    return physicalDevice.createDeviceUnique(vk::DeviceCreateInfo{
        {}, (uint32_t)m_QueueCreateInfos.size(), m_QueueCreateInfos.data(),
        (uint32_t)m_Layers.size(), m_Layers.data(),
        (uint32_t)m_DeviceExtensions.size(), m_DeviceExtensions.data() });
}



