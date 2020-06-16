#include "vulkan_instance.h"




//<======================== INSTANCE MAKER ==============================>
ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::setDefaultLayers()
{
    m_Layers.push_back("VK_LAYER_KHRONOS_validation");
#ifndef NDEBUG
    m_Extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
    return *this;
}

ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::addLayer(const char* layer)
{
    m_Layers.push_back(layer);
    return *this;
}

ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::addExtension(const char* layer)
{
    m_Extensions.push_back(layer);
    return *this;
}

ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::setApplicationName(const char* pApplicationName)
{
    m_AppInfo.pApplicationName = pApplicationName;
    return *this;
}

ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::setApplicationVersion(uint32 applicationVersion)
{
    m_AppInfo.applicationVersion = applicationVersion;
    return *this;
}

ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::setEngineName(const char* pEngineName)
{
    m_AppInfo.pEngineName = pEngineName;
    return *this;
}

ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::setEngineVersion(uint32 engineVersion)
{
    m_AppInfo.engineVersion = engineVersion;
    return *this;
}

ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::setApiVersion(uint32 apiVersion)
{
    m_AppInfo.apiVersion = apiVersion;
    return *this;
}

vk::UniqueInstance ngv::VulkanInstanceMaker::createUnique()
{
    return vk::createInstanceUnique(
        vk::InstanceCreateInfo{
          {}, &m_AppInfo, (uint32_t)m_Layers.size(),
          m_Layers.data(), (uint32_t)m_Extensions.size(),
          m_Extensions.data()
        }
    );
}


