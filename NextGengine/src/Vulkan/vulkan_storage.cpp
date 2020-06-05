#include "vulkan_storage.h"

#include "vulkan_allocator.h"




std::unique_ptr<ngv::VulkanImage> ngv::VulkanImage::make()
{
    return std::unique_ptr<VulkanImage>(new VulkanImage());
}

ngv::ImageBlockParams ngv::VulkanImage::getBlockParams(vk::Format format)
{
    switch (format) {
    case vk::Format::eR4G4UnormPack8: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eR4G4B4A4UnormPack16: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eB4G4R4A4UnormPack16: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR5G6B5UnormPack16: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eB5G6R5UnormPack16: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR5G5B5A1UnormPack16: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eB5G5R5A1UnormPack16: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eA1R5G5B5UnormPack16: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR8Unorm: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eR8Snorm: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eR8Uscaled: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eR8Sscaled: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eR8Uint: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eR8Sint: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eR8Srgb: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eR8G8Unorm: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Snorm: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Uscaled: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Sscaled: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Uint: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Sint: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Srgb: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8B8Unorm: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Snorm: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Uscaled: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Sscaled: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Uint: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Sint: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Srgb: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Unorm: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Snorm: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Uscaled: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Sscaled: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Uint: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Sint: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Srgb: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8A8Unorm: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Snorm: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Uscaled: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Sscaled: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Uint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Sint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Srgb: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Unorm: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Snorm: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Uscaled: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Sscaled: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Uint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Sint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Srgb: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8UnormPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8SnormPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8UscaledPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8SscaledPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8UintPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8SintPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8SrgbPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10UnormPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10SnormPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10UscaledPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10SscaledPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10UintPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10SintPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10UnormPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10SnormPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10UscaledPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10SscaledPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10UintPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10SintPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR16Unorm: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR16Snorm: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR16Uscaled: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR16Sscaled: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR16Uint: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR16Sint: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR16Sfloat: return ImageBlockParams{ 1, 1, 2 };
    case vk::Format::eR16G16Unorm: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Snorm: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Uscaled: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Sscaled: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Uint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Sint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Sfloat: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16B16Unorm: return ImageBlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Snorm: return ImageBlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Uscaled: return ImageBlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Sscaled: return ImageBlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Uint: return ImageBlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Sint: return ImageBlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Sfloat: return ImageBlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16A16Unorm: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Snorm: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Uscaled: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Sscaled: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Uint: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Sint: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Sfloat: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR32Uint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR32Sint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR32Sfloat: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eR32G32Uint: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR32G32Sint: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR32G32Sfloat: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR32G32B32Uint: return ImageBlockParams{ 1, 1, 12 };
    case vk::Format::eR32G32B32Sint: return ImageBlockParams{ 1, 1, 12 };
    case vk::Format::eR32G32B32Sfloat: return ImageBlockParams{ 1, 1, 12 };
    case vk::Format::eR32G32B32A32Uint: return ImageBlockParams{ 1, 1, 16 };
    case vk::Format::eR32G32B32A32Sint: return ImageBlockParams{ 1, 1, 16 };
    case vk::Format::eR32G32B32A32Sfloat: return ImageBlockParams{ 1, 1, 16 };
    case vk::Format::eR64Uint: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR64Sint: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR64Sfloat: return ImageBlockParams{ 1, 1, 8 };
    case vk::Format::eR64G64Uint: return ImageBlockParams{ 1, 1, 16 };
    case vk::Format::eR64G64Sint: return ImageBlockParams{ 1, 1, 16 };
    case vk::Format::eR64G64Sfloat: return ImageBlockParams{ 1, 1, 16 };
    case vk::Format::eR64G64B64Uint: return ImageBlockParams{ 1, 1, 24 };
    case vk::Format::eR64G64B64Sint: return ImageBlockParams{ 1, 1, 24 };
    case vk::Format::eR64G64B64Sfloat: return ImageBlockParams{ 1, 1, 24 };
    case vk::Format::eR64G64B64A64Uint: return ImageBlockParams{ 1, 1, 32 };
    case vk::Format::eR64G64B64A64Sint: return ImageBlockParams{ 1, 1, 32 };
    case vk::Format::eR64G64B64A64Sfloat: return ImageBlockParams{ 1, 1, 32 };
    case vk::Format::eB10G11R11UfloatPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eE5B9G9R9UfloatPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eD16Unorm: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eX8D24UnormPack32: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eD32Sfloat: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eS8Uint: return ImageBlockParams{ 1, 1, 1 };
    case vk::Format::eD16UnormS8Uint: return ImageBlockParams{ 1, 1, 3 };
    case vk::Format::eD24UnormS8Uint: return ImageBlockParams{ 1, 1, 4 };
    case vk::Format::eD32SfloatS8Uint: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eBc1RgbUnormBlock: return ImageBlockParams{ 4, 4, 8 };
    case vk::Format::eBc1RgbSrgbBlock: return ImageBlockParams{ 4, 4, 8 };
    case vk::Format::eBc1RgbaUnormBlock: return ImageBlockParams{ 4, 4, 8 };
    case vk::Format::eBc1RgbaSrgbBlock: return ImageBlockParams{ 4, 4, 8 };
    case vk::Format::eBc2UnormBlock: return ImageBlockParams{ 4, 4, 16 };
    case vk::Format::eBc2SrgbBlock: return ImageBlockParams{ 4, 4, 16 };
    case vk::Format::eBc3UnormBlock: return ImageBlockParams{ 4, 4, 16 };
    case vk::Format::eBc3SrgbBlock: return ImageBlockParams{ 4, 4, 16 };
    case vk::Format::eBc4UnormBlock: return ImageBlockParams{ 4, 4, 16 };
    case vk::Format::eBc4SnormBlock: return ImageBlockParams{ 4, 4, 16 };
    case vk::Format::eBc5UnormBlock: return ImageBlockParams{ 4, 4, 16 };
    case vk::Format::eBc5SnormBlock: return ImageBlockParams{ 4, 4, 16 };
    case vk::Format::eBc6HUfloatBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eBc6HSfloatBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eBc7UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eBc7SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8A1UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8A1SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8A8UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8A8SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEacR11UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEacR11SnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEacR11G11UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eEacR11G11SnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc4x4UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc4x4SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc5x4UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc5x4SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc5x5UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc5x5SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc6x5UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc6x5SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc6x6UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc6x6SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x5UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x5SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x6UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x6SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x8UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x8SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x5UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x5SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x6UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x6SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x8UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x8SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x10UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x10SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc12x10UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc12x10SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc12x12UnormBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::eAstc12x12SrgbBlock: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc12BppUnormBlockIMG: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc14BppUnormBlockIMG: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc22BppUnormBlockIMG: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc24BppUnormBlockIMG: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc12BppSrgbBlockIMG: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc14BppSrgbBlockIMG: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc22BppSrgbBlockIMG: return ImageBlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc24BppSrgbBlockIMG: return ImageBlockParams{ 0, 0, 0 };
    }
    return ImageBlockParams{ 0, 0, 0 };
}

uint32 ngv::VulkanImage::mipScale(uint32 value, uint32 mipLevel)
{
    return std::max(value >> mipLevel, (uint32_t)1);
}

vk::ImageCreateInfo ngv::VulkanImage::getImageCreateInfo()
{
    return m_ImageCreateInfo;
}

vk::MemoryPropertyFlags ngv::VulkanImage::getMemoryPropertyFlags()
{
    return m_MemoryPropertyFlags;
}

std::unique_ptr<ngv::VulkanBuffer> ngv::VulkanBuffer::make()
{
    return std::unique_ptr<VulkanBuffer>(new VulkanBuffer());
}

vk::BufferCreateInfo ngv::VulkanBuffer::getBufferCreateInfo()
{
    return m_BufferCreateInfo;
}

vk::MemoryPropertyFlags ngv::VulkanBuffer::getMemoryPropertyFlags()
{
    return m_MemoryPropertyFlags;
}

bool ngv::VulkanBuffer::hasAllocation()
{
    if (auto spt = m_pMemoryPage.lock()) {
        return true;
    }
    return false;
}

bool ngv::VulkanBuffer::hasSameAllocation(std::raw_ptr<VulkanBuffer> buffer)
{
    if (auto spt1 = m_pMemoryPage.lock()) {
        if (auto spt2 = buffer->m_pMemoryPage.lock()) {
            if (m_pAllocation == buffer->m_pAllocation) { // equiv to m_pAllocation.get() == buffer->m_pAllocation.get()
                return true;
            }
        }
    }
    return false;
}
