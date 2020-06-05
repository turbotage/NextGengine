#include "vulkan_utility.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>

uint32 ngv::findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memprops, uint32 memoryTypeBits, vk::MemoryPropertyFlags searchFlags)
{
    for (int i = 0; i != memprops.memoryTypeCount; ++i, memoryTypeBits >>= 1) {
        if (memoryTypeBits & 1) {
            if ((memprops.memoryTypes[i].propertyFlags & searchFlags) == searchFlags) {
                return i;
            }
        }
    }
    return -1;
}

uint32 ngv::mipScale(uint32 value, uint32 mipLevel)
{
    return std::max(value >> mipLevel, (uint32)1);
}

std::vector<uint8> ngv::loadFile(const std::string& filename)
{
    std::ifstream is(filename, std::ios::binary | std::ios::ate);
    std::vector<uint8_t> bytes;
    if (!is.fail()) {
        size_t size = is.tellg();
        is.seekg(0);
        bytes.resize(size);
        is.read((char*)bytes.data(), size);
    }
    return bytes;
}

ngv::BlockParams ngv::getBlockParams(vk::Format format)
{
    switch (format) {
    case vk::Format::eR4G4UnormPack8: return BlockParams{ 1, 1, 1 };
    case vk::Format::eR4G4B4A4UnormPack16: return BlockParams{ 1, 1, 2 };
    case vk::Format::eB4G4R4A4UnormPack16: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR5G6B5UnormPack16: return BlockParams{ 1, 1, 2 };
    case vk::Format::eB5G6R5UnormPack16: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR5G5B5A1UnormPack16: return BlockParams{ 1, 1, 2 };
    case vk::Format::eB5G5R5A1UnormPack16: return BlockParams{ 1, 1, 2 };
    case vk::Format::eA1R5G5B5UnormPack16: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR8Unorm: return BlockParams{ 1, 1, 1 };
    case vk::Format::eR8Snorm: return BlockParams{ 1, 1, 1 };
    case vk::Format::eR8Uscaled: return BlockParams{ 1, 1, 1 };
    case vk::Format::eR8Sscaled: return BlockParams{ 1, 1, 1 };
    case vk::Format::eR8Uint: return BlockParams{ 1, 1, 1 };
    case vk::Format::eR8Sint: return BlockParams{ 1, 1, 1 };
    case vk::Format::eR8Srgb: return BlockParams{ 1, 1, 1 };
    case vk::Format::eR8G8Unorm: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Snorm: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Uscaled: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Sscaled: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Uint: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Sint: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8Srgb: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR8G8B8Unorm: return BlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Snorm: return BlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Uscaled: return BlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Sscaled: return BlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Uint: return BlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Sint: return BlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8Srgb: return BlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Unorm: return BlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Snorm: return BlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Uscaled: return BlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Sscaled: return BlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Uint: return BlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Sint: return BlockParams{ 1, 1, 3 };
    case vk::Format::eB8G8R8Srgb: return BlockParams{ 1, 1, 3 };
    case vk::Format::eR8G8B8A8Unorm: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Snorm: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Uscaled: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Sscaled: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Uint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Sint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR8G8B8A8Srgb: return BlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Unorm: return BlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Snorm: return BlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Uscaled: return BlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Sscaled: return BlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Uint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Sint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eB8G8R8A8Srgb: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8UnormPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8SnormPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8UscaledPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8SscaledPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8UintPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8SintPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA8B8G8R8SrgbPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10UnormPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10SnormPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10UscaledPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10SscaledPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10UintPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2R10G10B10SintPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10UnormPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10SnormPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10UscaledPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10SscaledPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10UintPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eA2B10G10R10SintPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR16Unorm: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR16Snorm: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR16Uscaled: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR16Sscaled: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR16Uint: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR16Sint: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR16Sfloat: return BlockParams{ 1, 1, 2 };
    case vk::Format::eR16G16Unorm: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Snorm: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Uscaled: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Sscaled: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Uint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Sint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16Sfloat: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR16G16B16Unorm: return BlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Snorm: return BlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Uscaled: return BlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Sscaled: return BlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Uint: return BlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Sint: return BlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16Sfloat: return BlockParams{ 1, 1, 6 };
    case vk::Format::eR16G16B16A16Unorm: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Snorm: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Uscaled: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Sscaled: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Uint: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Sint: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR16G16B16A16Sfloat: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR32Uint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR32Sint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR32Sfloat: return BlockParams{ 1, 1, 4 };
    case vk::Format::eR32G32Uint: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR32G32Sint: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR32G32Sfloat: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR32G32B32Uint: return BlockParams{ 1, 1, 12 };
    case vk::Format::eR32G32B32Sint: return BlockParams{ 1, 1, 12 };
    case vk::Format::eR32G32B32Sfloat: return BlockParams{ 1, 1, 12 };
    case vk::Format::eR32G32B32A32Uint: return BlockParams{ 1, 1, 16 };
    case vk::Format::eR32G32B32A32Sint: return BlockParams{ 1, 1, 16 };
    case vk::Format::eR32G32B32A32Sfloat: return BlockParams{ 1, 1, 16 };
    case vk::Format::eR64Uint: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR64Sint: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR64Sfloat: return BlockParams{ 1, 1, 8 };
    case vk::Format::eR64G64Uint: return BlockParams{ 1, 1, 16 };
    case vk::Format::eR64G64Sint: return BlockParams{ 1, 1, 16 };
    case vk::Format::eR64G64Sfloat: return BlockParams{ 1, 1, 16 };
    case vk::Format::eR64G64B64Uint: return BlockParams{ 1, 1, 24 };
    case vk::Format::eR64G64B64Sint: return BlockParams{ 1, 1, 24 };
    case vk::Format::eR64G64B64Sfloat: return BlockParams{ 1, 1, 24 };
    case vk::Format::eR64G64B64A64Uint: return BlockParams{ 1, 1, 32 };
    case vk::Format::eR64G64B64A64Sint: return BlockParams{ 1, 1, 32 };
    case vk::Format::eR64G64B64A64Sfloat: return BlockParams{ 1, 1, 32 };
    case vk::Format::eB10G11R11UfloatPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eE5B9G9R9UfloatPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eD16Unorm: return BlockParams{ 1, 1, 4 };
    case vk::Format::eX8D24UnormPack32: return BlockParams{ 1, 1, 4 };
    case vk::Format::eD32Sfloat: return BlockParams{ 1, 1, 4 };
    case vk::Format::eS8Uint: return BlockParams{ 1, 1, 1 };
    case vk::Format::eD16UnormS8Uint: return BlockParams{ 1, 1, 3 };
    case vk::Format::eD24UnormS8Uint: return BlockParams{ 1, 1, 4 };
    case vk::Format::eD32SfloatS8Uint: return BlockParams{ 0, 0, 0 };
    case vk::Format::eBc1RgbUnormBlock: return BlockParams{ 4, 4, 8 };
    case vk::Format::eBc1RgbSrgbBlock: return BlockParams{ 4, 4, 8 };
    case vk::Format::eBc1RgbaUnormBlock: return BlockParams{ 4, 4, 8 };
    case vk::Format::eBc1RgbaSrgbBlock: return BlockParams{ 4, 4, 8 };
    case vk::Format::eBc2UnormBlock: return BlockParams{ 4, 4, 16 };
    case vk::Format::eBc2SrgbBlock: return BlockParams{ 4, 4, 16 };
    case vk::Format::eBc3UnormBlock: return BlockParams{ 4, 4, 16 };
    case vk::Format::eBc3SrgbBlock: return BlockParams{ 4, 4, 16 };
    case vk::Format::eBc4UnormBlock: return BlockParams{ 4, 4, 16 };
    case vk::Format::eBc4SnormBlock: return BlockParams{ 4, 4, 16 };
    case vk::Format::eBc5UnormBlock: return BlockParams{ 4, 4, 16 };
    case vk::Format::eBc5SnormBlock: return BlockParams{ 4, 4, 16 };
    case vk::Format::eBc6HUfloatBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eBc6HSfloatBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eBc7UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eBc7SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8A1UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8A1SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8A8UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEtc2R8G8B8A8SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEacR11UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEacR11SnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEacR11G11UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eEacR11G11SnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc4x4UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc4x4SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc5x4UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc5x4SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc5x5UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc5x5SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc6x5UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc6x5SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc6x6UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc6x6SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x5UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x5SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x6UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x6SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x8UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc8x8SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x5UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x5SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x6UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x6SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x8UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x8SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x10UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc10x10SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc12x10UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc12x10SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc12x12UnormBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::eAstc12x12SrgbBlock: return BlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc12BppUnormBlockIMG: return BlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc14BppUnormBlockIMG: return BlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc22BppUnormBlockIMG: return BlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc24BppUnormBlockIMG: return BlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc12BppSrgbBlockIMG: return BlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc14BppSrgbBlockIMG: return BlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc22BppSrgbBlockIMG: return BlockParams{ 0, 0, 0 };
    case vk::Format::ePvrtc24BppSrgbBlockIMG: return BlockParams{ 0, 0, 0 };
    }
    return BlockParams{ 0, 0, 0 };
}




//<============================== SHADER-MODULE MAKER ======================>
//public
ngv::VulkanShaderModule::VulkanShaderModule(const vk::Device& device, const std::string& filename)
{
    auto file = std::ifstream(filename, std::ios::binary);
    if (file.bad()) {
        return;
    }

    file.seekg(0, std::ios::end);
    int length = (int)file.tellg();

    m_State.opcodes.resize((size_t)(length / 4));
    file.seekg(0, std::ios::beg);
    file.read((char*)m_State.opcodes.data(), m_State.opcodes.size() * 4);

    vk::ShaderModuleCreateInfo ci;
    ci.codeSize = m_State.opcodes.size() * 4;
    ci.pCode = m_State.opcodes.data();
    m_State.module = device.createShaderModuleUnique(ci);

    m_State.ok = true;
}

std::vector<ngv::VulkanShaderModule::Variable> ngv::VulkanShaderModule::getVariables() const
{
    auto bound = m_State.opcodes[3];

    std::unordered_map<int, int> bindings;
    std::unordered_map<int, int> locations;
    std::unordered_map<int, int> sets;
    std::unordered_map<int, std::string> debugNames;

    for (int i = 5; i != m_State.opcodes.size(); i += m_State.opcodes[i] >> 16) {
        spv::Op op = spv::Op(m_State.opcodes[i] & 0xffff);
        if (op == spv::Op::OpDecorate) {
            int name = m_State.opcodes[i + 1];
            auto decoration = spv::Decoration(m_State.opcodes[i + 2]);
            if (decoration == spv::Decoration::Binding) {
                bindings[name] = m_State.opcodes[i + 3];
            }
            else if (decoration == spv::Decoration::Location) {
                locations[name] = m_State.opcodes[i + 3];
            }
            else if (decoration == spv::Decoration::DescriptorSet) {
                sets[name] = m_State.opcodes[i + 3];
            }
        }
        else if (op == spv::Op::OpName) {
            int name = m_State.opcodes[i + 1];
            debugNames[name] = (const char*)&m_State.opcodes[i + 2];
        }
    }

    std::vector<Variable> result;
    for (int i = 5; i != m_State.opcodes.size(); i += m_State.opcodes[i] >> 16) {
        spv::Op op = spv::Op(m_State.opcodes[i] & 0xffff);
        if (op == spv::Op::OpVariable) {
            int name = m_State.opcodes[i + 1];
            auto sc = spv::StorageClass(m_State.opcodes[i + 3]);
            Variable b;
            b.debugName = debugNames[name];
            b.name = name;
            b.location = locations[name];
            b.set = sets[name];
            b.instruction = i;
            b.storageClass = sc;
            result.push_back(b);
        }
    }
    return result;
}

bool ngv::VulkanShaderModule::ok() const
{
    return *m_State.module;
}

vk::ShaderModule ngv::VulkanShaderModule::module()
{
    return *m_State.module;
}

std::ostream& ngv::VulkanShaderModule::write(std::ostream& os)
{
    os << "static const uint32_t shader[] = {\n";
    char tmp[256];
    auto p = m_State.opcodes.begin();
    snprintf(
        tmp, sizeof(tmp), "  0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,\n", p[0], p[1], p[2], p[3], p[4]);
    os << tmp;
    for (int i = 5; i != m_State.opcodes.size(); i += m_State.opcodes[i] >> 16) {
        char* p = tmp + 2, * e = tmp + sizeof(tmp) - 2;
        for (int j = i; j != i + (m_State.opcodes[i] >> 16); ++j) {
            p += snprintf(p, e - p, "0x%08x,", m_State.opcodes[j]);
            if (p > e - 16) { *p++ = '\n'; *p = 0; os << tmp; p = tmp + 2; }
        }
        *p++ = '\n';
        *p = 0;
        os << tmp;
    }
    os << "};\n\n";
    return os;
}


