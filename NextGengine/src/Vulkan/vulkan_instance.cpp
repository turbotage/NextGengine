#include "vulkan_instance.h"



//<======================== INSTANCE MAKER ==============================>
ngv::VulkanInstanceMaker& ngv::VulkanInstanceMaker::setDefaultLayers()
{
    m_Layers.push_back("VK_LAYER_LUNARG_standard_validation");
    m_Extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#ifdef VKU_SURFACE
    m_Extensions_.push_back(VKU_SURFACE);
#endif
    m_Extensions.push_back("VK_KHR_surface");
#if defined( __APPLE__ ) && defined(VK_EXT_METAL_SURFACE_EXTENSION_NAME)
    m_Extensions_.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif //__APPLE__
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


