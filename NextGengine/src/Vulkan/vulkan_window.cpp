#include "vulkan_window.h"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "vulkandef.h"
#include "vulkan_device.h"
#include "vulkan_storage.h"
#include "vulkan_allocator.h"
#include "vulkan_renderpass.h"

#include <vector>
#include <string>
#include <iostream>

ngv::VulkanWindow::VulkanWindow(const vk::Instance& instance, VulkanDevice& device, VulkanAllocator& allocator, uint32 graphicsQueueFamilyIndex, GLFWwindow* pWindow)
	: m_Device(device), m_Allocator(allocator)
{
	m_Instance = instance;
	m_GraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;

	glfwCreateWindowSurface(instance, pWindow,
		nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_Surface));

	init();
}

void ngv::VulkanWindow::dumpCaps(std::ostream& os, vk::PhysicalDevice pd) const
{
	os << "Surface formats\n";
	auto fmts = pd.getSurfaceFormatsKHR(m_Surface);
	for (auto& fmt : fmts) {
		auto fmtstr = vk::to_string(fmt.format);
		auto cstr = vk::to_string(fmt.colorSpace);
		os << "format=" << fmtstr << " colorSpace=" << cstr << "\n";
	}

	os << "Present Modes\n";
	auto presentModes = pd.getSurfacePresentModesKHR(m_Surface);
	for (auto pm : presentModes) {
		os << vk::to_string(pm) << "\n";
	}
}

void ngv::VulkanWindow::draw(const vk::Queue& graphicsQueue, const std::function<void(vk::CommandBuffer cb, uint32 imageIndex, vk::RenderPassBeginInfo& rpbi)>& renderFunc)
{
	//?????
	static auto start = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::high_resolution_clock::now();
	auto delta = time - start;
	start = time;
	
	auto umax = (uint64)std::numeric_limits<uint64>::max;
	uint32 imageIndex;
	m_Device.device().acquireNextImageKHR(*m_Swapchain, umax, *m_ImageAcquireSemaphore, vk::Fence(), &imageIndex);

	vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::Semaphore ccSema = *m_CommandCompleteSemaphore;
	vk::Semaphore iaSema = *m_ImageAcquireSemaphore;

	vk::CommandBuffer cb = *m_CommandBuffers[imageIndex];

	vk::Fence cbFence = m_CommandBufferFences[imageIndex];
	m_Device.device().waitForFences(cbFence, 1, umax);
	m_Device.device().resetFences(cbFence);

	std::array<float, 4> clearColorValue{ 0.75f, 0.75f, 0.75f, 1 };
	vk::ClearDepthStencilValue clearDepthValue{ 1.0f, 0 };
	std::array<vk::ClearValue, 2> clearColors{ vk::ClearValue{clearColorValue}, clearDepthValue };
	vk::RenderPassBeginInfo rpbi;
	rpbi.renderPass = *m_RenderPass;
	rpbi.framebuffer = *m_Framebuffers[imageIndex];
	rpbi.renderArea = vk::Rect2D{ {0,0}, {m_Width, m_Height} };
	rpbi.clearValueCount = clearColors.size();
	rpbi.pClearValues = clearColors.data();

	renderFunc(cb, imageIndex, rpbi);

	vk::SubmitInfo submit;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &iaSema;
	submit.pWaitDstStageMask = &waitStages;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cb;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &ccSema;
	graphicsQueue.submit(1, &submit, cbFence);

	vk::PresentInfoKHR presentInfo;
	vk::SwapchainKHR swapchain = *m_Swapchain;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &ccSema;
	

}

uint32 ngv::VulkanWindow::presentQueueFamilyIndex() const
{
	return m_PresentQueueFamilyIndex;
}

const vk::Queue ngv::VulkanWindow::presentQueue() const
{
	return m_Device.device().getQueue(m_PresentQueueFamilyIndex, 0);
}

bool ngv::VulkanWindow::ok() const
{
	return m_Ok;
}

vk::RenderPass ngv::VulkanWindow::renderPass() const
{
	return *m_RenderPass;
}

const std::vector<vk::UniqueFramebuffer>& ngv::VulkanWindow::framebuffers() const
{
	return m_Framebuffers;
}

ngv::VulkanWindow::~VulkanWindow()
{
	for (auto& iv : m_SwapChainViews) {
		m_Device.device().destroyImageView(iv);
	}
	for (auto& f : m_CommandBufferFences) {
		m_Device.device().destroyFence(f);
	}
	m_Swapchain.reset(); //m_Swapchain = vk::UniqueSwapchainKHR{};
}

uint32 ngv::VulkanWindow::width() const
{
	return m_Width;
}

uint32 ngv::VulkanWindow::height() const
{
	return m_Height;
}

vk::Format ngv::VulkanWindow::swapchainImageFormat() const
{
	return m_SwapChainImageFormat;
}

vk::ColorSpaceKHR ngv::VulkanWindow::swapchainColorSpace() const
{
	return m_SwapChainColorSpace;
}

const vk::SwapchainKHR ngv::VulkanWindow::swapchain() const
{
	return *m_Swapchain;
}

const std::vector<vk::ImageView>& ngv::VulkanWindow::imageViews() const
{
	return m_SwapChainViews;
}

const std::vector<vk::Image>& ngv::VulkanWindow::images() const
{
	return m_SwapChainImages;
}

const std::vector<vk::UniqueCommandBuffer>& ngv::VulkanWindow::commandBuffers() const
{
	return m_CommandBuffers;
}

const std::vector<vk::Fence>& ngv::VulkanWindow::commandBufferFences() const
{
	return m_CommandBufferFences;
}

vk::Semaphore ngv::VulkanWindow::imageAcquireSemaphore() const
{
	return *m_ImageAcquireSemaphore;
}

vk::Semaphore ngv::VulkanWindow::commandCompleteSemaphore() const
{
	return *m_CommandCompleteSemaphore;
}

vk::CommandPool ngv::VulkanWindow::commandPool() const
{
	return *m_CommandPool;
}

uint32 ngv::VulkanWindow::numSwapchainImages() const
{
	return m_SwapChainImages.size();
}

//private
void ngv::VulkanWindow::init()
{
	
	auto& pd = m_Device.physicalDevice();
	auto qprops = pd.getQueueFamilyProperties();
	bool found = false;
	for (uint32 qi = 0; qi != qprops.size(); ++qi) {
		auto& qprop = qprops[qi];
		if (pd.getSurfaceSupportKHR(qi, m_Surface) && 
			(qprop.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics) {
			m_PresentQueueFamilyIndex = qi;
			found = true;
			break;
		}
	}

	if (!found) {
		std::cout << "No Vulkan present queues found\n";
		return;
	}

	auto fmts = pd.getSurfaceFormatsKHR(m_Surface);
	m_SwapChainImageFormat = fmts[0].format;
	m_SwapChainColorSpace = fmts[0].colorSpace;
	if (fmts.size() == 1 && m_SwapChainImageFormat == vk::Format::eUndefined) {
		m_SwapChainImageFormat = vk::Format::eB8G8R8A8Unorm;
		m_SwapChainColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	}
	else {
		for (auto& fmt : fmts) {
			if (fmt.format == vk::Format::eB8G8R8A8Unorm) {
				m_SwapChainImageFormat = fmt.format;
				m_SwapChainColorSpace = fmt.colorSpace;
			}
		}
	}

	auto surfaceCaps = pd.getSurfaceCapabilitiesKHR(m_Surface);
	m_Width = surfaceCaps.currentExtent.width;
	m_Height = surfaceCaps.currentExtent.height;

	auto pms = pd.getSurfacePresentModesKHR(m_Surface);
	vk::PresentModeKHR presentMode = pms[0];
	if (std::find(pms.begin(), pms.end(), vk::PresentModeKHR::eFifo) != pms.end()) {
		presentMode = vk::PresentModeKHR::eFifo;
	}
	else {
		std::cout << "No fifo mode available\n";
		return;
	}

	vk::SwapchainCreateInfoKHR swapinfo{};
	std::array<uint32_t, 2> queueFamilyIndices = { m_GraphicsQueueFamilyIndex, m_PresentQueueFamilyIndex };
	bool sameQueues = queueFamilyIndices[0] == queueFamilyIndices[1];
	vk::SharingMode sharingMode = !sameQueues ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
	swapinfo.imageExtent = surfaceCaps.currentExtent;
	swapinfo.surface = m_Surface;
	swapinfo.minImageCount = surfaceCaps.minImageCount + 1;
	swapinfo.imageFormat = m_SwapChainImageFormat;
	swapinfo.imageColorSpace = m_SwapChainColorSpace;
	swapinfo.imageExtent = surfaceCaps.currentExtent;
	swapinfo.imageArrayLayers = 1;
	swapinfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapinfo.imageSharingMode = sharingMode;
	swapinfo.queueFamilyIndexCount = !sameQueues ? 2 : 0;
	swapinfo.pQueueFamilyIndices = queueFamilyIndices.data();
	swapinfo.preTransform = surfaceCaps.currentTransform;;
	swapinfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapinfo.presentMode = presentMode;
	swapinfo.clipped = 1;
	swapinfo.oldSwapchain = vk::SwapchainKHR{};
	m_Swapchain = m_Device.device().createSwapchainKHRUnique(swapinfo);

	m_SwapChainImages = m_Device.device().getSwapchainImagesKHR(*m_Swapchain);
	for (auto& img : m_SwapChainImages) {
		vk::ImageViewCreateInfo ci{};
		ci.image = img;
		ci.viewType = vk::ImageViewType::e2D;
		ci.format = m_SwapChainImageFormat;
		m_SwapChainViews.emplace_back(m_Device.device().createImageView(ci));
	}

	auto memprops = m_Device.physicalDevice().getMemoryProperties();
	//fix depth stencil // CRASHES HERE, broken allocator? probs
	m_DepthStencil = DepthStencilImage::make(m_Device, m_Width, m_Height, vk::Format::eD24UnormS8Uint);
	m_Allocator.giveImageAllocation(m_DepthStencil);

	ngv::VulkanRenderpassMaker rpm{};

	// The only color attachmnet
	rpm.beginAttachment(m_SwapChainImageFormat);
	rpm.setAttachmentLoadOp(vk::AttachmentLoadOp::eClear);
	rpm.setAttachmentStoreOp(vk::AttachmentStoreOp::eStore);
	rpm.setAttachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	// The depth/stencil attachment
	rpm.beginAttachment(m_DepthStencil->format());
	rpm.setAttachmentLoadOp(vk::AttachmentLoadOp::eClear);
	rpm.setAttachmentStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	rpm.setAttachmentFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);


	// A subpass to render using the above attachments
	rpm.beginSubpass(vk::PipelineBindPoint::eGraphics);
	rpm.addSubpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0);
	rpm.setSubpassDepthStencilAttachment(vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);

	// A dependency to reset the layout of both attachments.
	rpm.beginDependency(VK_SUBPASS_EXTERNAL, 0);
	rpm.setDependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	rpm.setDependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	rpm.setDependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	m_RenderPass = rpm.createUnique(m_Device.device());

	for (int i = 0; i != m_SwapChainViews.size(); ++i) {
		vk::ImageView attachments[2] = { m_SwapChainViews[i], m_DepthStencil->imageView() };
		vk::FramebufferCreateInfo fbci{ {}, *m_RenderPass, 2, attachments, m_Width, m_Height, 1 };
		m_Framebuffers.push_back(m_Device.device().createFramebufferUnique(fbci));
	}

	vk::SemaphoreCreateInfo sci;
	m_ImageAcquireSemaphore = m_Device.device().createSemaphoreUnique(sci);
	m_CommandCompleteSemaphore = m_Device.device().createSemaphoreUnique(sci);

	typedef vk::CommandPoolCreateFlagBits ccbits;
	vk::CommandPoolCreateInfo cpci{ ccbits::eTransient | ccbits::eResetCommandBuffer, m_GraphicsQueueFamilyIndex };
	m_CommandPool = m_Device.device().createCommandPoolUnique(cpci);

	// Create static draw buffers;
	vk::CommandBufferAllocateInfo cbai{ *m_CommandPool, vk::CommandBufferLevel::ePrimary, (uint32)m_Framebuffers.size() };
	m_CommandBuffers = m_Device.device().allocateCommandBuffersUnique(cbai);

	for (int i = 0; i != m_CommandBuffers.size(); ++i) {
		vk::FenceCreateInfo fci;
		fci.flags = vk::FenceCreateFlagBits::eSignaled;
		m_CommandBufferFences.emplace_back(m_Device.device().createFence(fci));
	}
	
	//TODO: consider adding static draw buffers

	m_Ok = true;
}
 