#include "vulkan_pipeline.h"

#include "vulkan_utility.h"


//<================== PIPELINE LAYOUT MAKER ======================>
//public
vk::UniquePipelineLayout ngv::VulkanPipelineLayoutMaker::createUnique(const vk::Device& device) const
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        {}, (uint32_t)m_SetLayouts.size(),
        m_SetLayouts.data(), (uint32_t)m_PushConstantRanges.size(),
        m_PushConstantRanges.data() };
    return device.createPipelineLayoutUnique(pipelineLayoutInfo);
}

void ngv::VulkanPipelineLayoutMaker::addDescriptorSetLayout(vk::DescriptorSetLayout layout)
{
    m_SetLayouts.push_back(layout);
}

void ngv::VulkanPipelineLayoutMaker::addPushConstantRange(vk::ShaderStageFlags stageFlags, uint32 offset, uint32 size)
{
    m_PushConstantRanges.emplace_back(stageFlags, offset, size);
}






//<======================= PIPELINE MAKER ============================>
//public
ngv::VulkanPipelineMaker::VulkanPipelineMaker(uint32 width, uint32 height)
{
    m_Viewport = vk::Viewport{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    m_Scissor = vk::Rect2D{ {0, 0}, {width, height} };

    m_InputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
    m_RasterizationState.lineWidth = 1.0f;

    // Set up depth test, but do not enable it.
    m_DepthStencilState.depthTestEnable = VK_FALSE;
    m_DepthStencilState.depthWriteEnable = VK_TRUE;
    m_DepthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
    m_DepthStencilState.depthBoundsTestEnable = VK_FALSE;
    m_DepthStencilState.back.failOp = vk::StencilOp::eKeep;
    m_DepthStencilState.back.passOp = vk::StencilOp::eKeep;
    m_DepthStencilState.back.compareOp = vk::CompareOp::eAlways;
    m_DepthStencilState.stencilTestEnable = VK_FALSE;
    m_DepthStencilState.front = m_DepthStencilState.back;
}

vk::UniquePipeline ngv::VulkanPipelineMaker::createUnique(const vk::Device& device, const vk::PipelineCache& pipelineCache, const vk::PipelineLayout& pipelineLayout, const vk::RenderPass& renderPass, bool defaultBlend)
{
    // Add default colour blend attachment if necessary.
    if (m_ColorBlendAttachments.empty() && defaultBlend) {
        vk::PipelineColorBlendAttachmentState blend{};
        blend.blendEnable = 0;
        blend.srcColorBlendFactor = vk::BlendFactor::eOne;
        blend.dstColorBlendFactor = vk::BlendFactor::eZero;
        blend.colorBlendOp = vk::BlendOp::eAdd;
        blend.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        blend.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        blend.alphaBlendOp = vk::BlendOp::eAdd;
        typedef vk::ColorComponentFlagBits ccbf;
        blend.colorWriteMask = ccbf::eR | ccbf::eG | ccbf::eB | ccbf::eA;
        m_ColorBlendAttachments.push_back(blend);
    }

    auto count = (uint32_t)m_ColorBlendAttachments.size();
    m_ColorBlendState.attachmentCount = count;
    m_ColorBlendState.pAttachments = count ? m_ColorBlendAttachments.data() : nullptr;

    vk::PipelineViewportStateCreateInfo viewportState{
        {}, 1, &m_Viewport, 1, &m_Scissor };

    vk::PipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.vertexAttributeDescriptionCount = (uint32_t)m_VertexAttributeDescriptions.size();
    vertexInputState.pVertexAttributeDescriptions = m_VertexAttributeDescriptions.data();
    vertexInputState.vertexBindingDescriptionCount = (uint32_t)m_VertexBindingDescriptions.size();
    vertexInputState.pVertexBindingDescriptions = m_VertexBindingDescriptions.data();

    vk::PipelineDynamicStateCreateInfo dynState{ {}, (uint32_t)m_DynamicState.size(), m_DynamicState.data() };

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.stageCount = (uint32_t)m_Modules.size();
    pipelineInfo.pStages = m_Modules.data();
    pipelineInfo.pInputAssemblyState = &m_InputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &m_RasterizationState;
    pipelineInfo.pMultisampleState = &m_MultisampleState;
    pipelineInfo.pColorBlendState = &m_ColorBlendState;
    pipelineInfo.pDepthStencilState = &m_DepthStencilState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pDynamicState = m_DynamicState.empty() ? nullptr : &dynState;
    pipelineInfo.subpass = m_Subpass;

    return device.createGraphicsPipelineUnique(pipelineCache, pipelineInfo);
}

void ngv::VulkanPipelineMaker::addShader(vk::ShaderStageFlagBits stage, VulkanShaderModule& shader, const char* entryPoint)
{
    vk::PipelineShaderStageCreateInfo info{};
    info.module = shader.module();
    info.pName = entryPoint;
    info.stage = stage;
    m_Modules.emplace_back(info);
}

void ngv::VulkanPipelineMaker::addColorBlend(const vk::PipelineColorBlendAttachmentState& state)
{
    m_ColorBlendAttachments.push_back(state);
}

void ngv::VulkanPipelineMaker::addSubPass(uint32 subpass)
{
    m_Subpass = subpass;
}

void ngv::VulkanPipelineMaker::blendBegin(vk::Bool32 enable)
{
    m_ColorBlendAttachments.emplace_back();
    auto& blend = m_ColorBlendAttachments.back();
    blend.blendEnable = enable;
    blend.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    blend.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    blend.colorBlendOp = vk::BlendOp::eAdd;
    blend.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
    blend.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    blend.alphaBlendOp = vk::BlendOp::eAdd;
    typedef vk::ColorComponentFlagBits ccbf;
    blend.colorWriteMask = ccbf::eR | ccbf::eG | ccbf::eB | ccbf::eA;
}

void ngv::VulkanPipelineMaker::blendEnable(vk::Bool32 value)
{
    m_ColorBlendAttachments.back().blendEnable = value;
}

void ngv::VulkanPipelineMaker::blendSrcColorBlendFactor(vk::BlendFactor value)
{
    m_ColorBlendAttachments.back().srcColorBlendFactor = value;
}

void ngv::VulkanPipelineMaker::blendDstColorBlendFactor(vk::BlendFactor value)
{
    m_ColorBlendAttachments.back().dstColorBlendFactor = value;
}

void ngv::VulkanPipelineMaker::blendColorBlendOp(vk::BlendOp value)
{
    m_ColorBlendAttachments.back().colorBlendOp = value;
}

void ngv::VulkanPipelineMaker::blendSrcAlphaBlendFactor(vk::BlendFactor value)
{
    m_ColorBlendAttachments.back().srcAlphaBlendFactor = value;
}

void ngv::VulkanPipelineMaker::blendDstAlphaBlendFactor(vk::BlendFactor value)
{
    m_ColorBlendAttachments.back().dstAlphaBlendFactor = value;
}

void ngv::VulkanPipelineMaker::blendAlphaBlendOp(vk::BlendOp value)
{
    m_ColorBlendAttachments.back().alphaBlendOp = value;
}

void ngv::VulkanPipelineMaker::blendColorWriteMask(vk::ColorComponentFlags value)
{
    m_ColorBlendAttachments.back().colorWriteMask = value;
}

void ngv::VulkanPipelineMaker::vertexAttribute(uint32_t location, uint32_t binding, vk::Format format, uint32_t offset)
{
    m_VertexAttributeDescriptions.push_back({ location, binding, format, offset });
}

void ngv::VulkanPipelineMaker::vertexAttribute(const vk::VertexInputAttributeDescription& desc)
{
    m_VertexAttributeDescriptions.push_back(desc);
}

void ngv::VulkanPipelineMaker::vertexBinding(uint32_t binding, uint32_t stride, vk::VertexInputRate inputRate)
{
    m_VertexBindingDescriptions.push_back({ binding, stride, inputRate });
}

void ngv::VulkanPipelineMaker::vertexBinding(const vk::VertexInputBindingDescription& desc)
{
    m_VertexBindingDescriptions.push_back(desc);
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::topology(vk::PrimitiveTopology topology)
{
    m_InputAssemblyState.topology = topology; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::primitiveRestartEnable(vk::Bool32 primitiveRestartEnable)
{
    m_InputAssemblyState.primitiveRestartEnable = primitiveRestartEnable; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::inputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo& value)
{
    m_InputAssemblyState = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::viewport(const vk::Viewport& value)
{
    m_Viewport = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::scissor(const vk::Rect2D& value)
{
    m_Scissor = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::rasterizationState(const vk::PipelineRasterizationStateCreateInfo& value)
{
    m_RasterizationState = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthClampEnable(vk::Bool32 value)
{
    m_RasterizationState.depthClampEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::rasterizerDiscardEnable(vk::Bool32 value)
{
    m_RasterizationState.rasterizerDiscardEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::polygonMode(vk::PolygonMode value)
{
    m_RasterizationState.polygonMode = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::cullMode(vk::CullModeFlags value)
{
    m_RasterizationState.cullMode = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::frontFace(vk::FrontFace value)
{
    m_RasterizationState.frontFace = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthBiasEnable(vk::Bool32 value)
{
    m_RasterizationState.depthBiasEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthBiasConstantFactor(float value)
{
    m_RasterizationState.depthBiasConstantFactor = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthBiasClamp(float value)
{
    m_RasterizationState.depthBiasClamp = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthBiasSlopeFactor(float value)
{
    m_RasterizationState.depthBiasSlopeFactor = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::lineWidth(float value)
{
    m_RasterizationState.lineWidth = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::multisampleState(const vk::PipelineMultisampleStateCreateInfo& value)
{
    m_MultisampleState = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::rasterizationSamples(vk::SampleCountFlagBits value)
{
    m_MultisampleState.rasterizationSamples = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::sampleShadingEnable(vk::Bool32 value)
{
    m_MultisampleState.sampleShadingEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::minSampleShading(float value)
{
    m_MultisampleState.minSampleShading = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::pSampleMask(const vk::SampleMask* value)
{
    m_MultisampleState.pSampleMask = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::alphaToCoverageEnable(vk::Bool32 value)
{
    m_MultisampleState.alphaToCoverageEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::alphaToOneEnable(vk::Bool32 value)
{
    m_MultisampleState.alphaToOneEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthStencilState(const vk::PipelineDepthStencilStateCreateInfo& value)
{
    m_DepthStencilState = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthTestEnable(vk::Bool32 value)
{
    m_DepthStencilState.depthTestEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthWriteEnable(vk::Bool32 value)
{
    m_DepthStencilState.depthWriteEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthCompareOp(vk::CompareOp value)
{
    m_DepthStencilState.depthCompareOp = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::depthBoundsTestEnable(vk::Bool32 value)
{
    m_DepthStencilState.depthBoundsTestEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::stencilTestEnable(vk::Bool32 value)
{
    m_DepthStencilState.stencilTestEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::front(vk::StencilOpState value)
{
    m_DepthStencilState.front = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::back(vk::StencilOpState value)
{
    m_DepthStencilState.back = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::minDepthBounds(float value)
{
    m_DepthStencilState.minDepthBounds = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::maxDepthBounds(float value)
{
    m_DepthStencilState.maxDepthBounds = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::colorBlendState(const vk::PipelineColorBlendStateCreateInfo& value)
{
    m_ColorBlendState = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::logicOpEnable(vk::Bool32 value)
{
    m_ColorBlendState.logicOpEnable = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::logicOp(vk::LogicOp value)
{
    m_ColorBlendState.logicOp = value; return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::blendConstants(float r, float g, float b, float a)
{
    float* bc = m_ColorBlendState.blendConstants; bc[0] = r; bc[1] = g; bc[2] = b; bc[3] = a;
    return *this;
}

ngv::VulkanPipelineMaker& ngv::VulkanPipelineMaker::dynamicState(vk::DynamicState value)
{
    m_DynamicState.push_back(value); return *this;
}