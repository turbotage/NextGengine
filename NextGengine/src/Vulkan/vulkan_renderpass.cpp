#include "vulkan_renderpass.h"



//<======================== RENDERPASS MAKER ===============================>
//public

void ngv::VulkanRenderpassMaker::beginAttachment(vk::Format format)
{
    vk::AttachmentDescription desc{ {}, format };
    m_State.attachmentDescriptions.push_back(desc);
}

void ngv::VulkanRenderpassMaker::setAttachmentFlags(vk::AttachmentDescriptionFlags value)
{
    m_State.attachmentDescriptions.back().flags = value;
}

void ngv::VulkanRenderpassMaker::setAttachmentFormat(vk::Format value)
{
    m_State.attachmentDescriptions.back().format = value;
}

void ngv::VulkanRenderpassMaker::setAttachmentSamples(vk::SampleCountFlagBits value)
{
    m_State.attachmentDescriptions.back().samples = value;
}

void ngv::VulkanRenderpassMaker::setAttachmentLoadOp(vk::AttachmentLoadOp value)
{
    m_State.attachmentDescriptions.back().loadOp = value;
}

void ngv::VulkanRenderpassMaker::setAttachmentStoreOp(vk::AttachmentStoreOp value)
{
    m_State.attachmentDescriptions.back().storeOp = value;
}

void ngv::VulkanRenderpassMaker::setAttachmentStencilLoadOp(vk::AttachmentLoadOp value)
{
    m_State.attachmentDescriptions.back().stencilLoadOp = value;
}

void ngv::VulkanRenderpassMaker::setAttachmentStencilStoreOp(vk::AttachmentStoreOp value)
{
    m_State.attachmentDescriptions.back().stencilStoreOp = value;
}

void ngv::VulkanRenderpassMaker::setAttachmentInitialLayout(vk::ImageLayout value)
{
    m_State.attachmentDescriptions.back().initialLayout = value;
}

void ngv::VulkanRenderpassMaker::setAttachmentFinalLayout(vk::ImageLayout value)
{
    m_State.attachmentDescriptions.back().finalLayout = value;
}

void ngv::VulkanRenderpassMaker::beginSubpass(vk::PipelineBindPoint bp)
{
    vk::SubpassDescription desc{};
    desc.pipelineBindPoint = bp;
    m_State.subpassDescriptions.push_back(desc);
}

void ngv::VulkanRenderpassMaker::addSubpassColorAttachment(vk::ImageLayout layout, uint32_t attachment)
{
    vk::SubpassDescription& subpass = m_State.subpassDescriptions.back();
    auto* p = getAttachmentReference();
    p->layout = layout;
    p->attachment = attachment;
    if (subpass.colorAttachmentCount == 0) {
        subpass.pColorAttachments = p;
    }
    subpass.colorAttachmentCount++;
}

void ngv::VulkanRenderpassMaker::setSubpassDepthStencilAttachment(vk::ImageLayout layout, uint32_t attachment)
{
    vk::SubpassDescription& subpass = m_State.subpassDescriptions.back();
    auto* p = getAttachmentReference();
    p->layout = layout;
    p->attachment = attachment;
    subpass.pDepthStencilAttachment = p;
}

vk::UniqueRenderPass ngv::VulkanRenderpassMaker::createUnique(const vk::Device& device) const
{
    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = (uint32_t)m_State.attachmentDescriptions.size();
    renderPassInfo.pAttachments = m_State.attachmentDescriptions.data();
    renderPassInfo.subpassCount = (uint32_t)m_State.subpassDescriptions.size();
    renderPassInfo.pSubpasses = m_State.subpassDescriptions.data();
    renderPassInfo.dependencyCount = (uint32_t)m_State.subpassDependencies.size();
    renderPassInfo.pDependencies = m_State.subpassDependencies.data();
    return device.createRenderPassUnique(renderPassInfo);
}

void ngv::VulkanRenderpassMaker::beginDependency(uint32_t srcSubpass, uint32_t dstSubpass)
{
    vk::SubpassDependency desc{};
    desc.srcSubpass = srcSubpass;
    desc.dstSubpass = dstSubpass;
    m_State.subpassDependencies.push_back(desc);
}

void ngv::VulkanRenderpassMaker::setDependencySrcSubpass(uint32_t value)
{
    m_State.subpassDependencies.back().srcSubpass = value;
}

void ngv::VulkanRenderpassMaker::setDependencyDstSubpass(uint32_t value)
{
    m_State.subpassDependencies.back().dstSubpass = value;
}

void ngv::VulkanRenderpassMaker::setDependencySrcStageMask(vk::PipelineStageFlags value)
{
    m_State.subpassDependencies.back().srcStageMask = value;
}

void ngv::VulkanRenderpassMaker::setDependencyDstStageMask(vk::PipelineStageFlags value)
{
    m_State.subpassDependencies.back().dstStageMask = value;
}

void ngv::VulkanRenderpassMaker::setDependencySrcAccessMask(vk::AccessFlags value)
{
    m_State.subpassDependencies.back().srcAccessMask = value;
}

void ngv::VulkanRenderpassMaker::setDependencyDstAccessMask(vk::AccessFlags value)
{
    m_State.subpassDependencies.back().dstAccessMask = value;
}

void ngv::VulkanRenderpassMaker::setDependencyDependencyFlags(vk::DependencyFlags value)
{
    m_State.subpassDependencies.back().dependencyFlags = value;
}

//private
vk::AttachmentReference* ngv::VulkanRenderpassMaker::getAttachmentReference()
{
    return (m_State.numRefs < m_MaxRefs) ? &m_State.attachmentReferences[m_State.numRefs++] : nullptr;
}


