#include "vulkan_window.h"

#include "vulkandef.h"
#include "vulkan_device.h"
#include "vulkan_storage.h"
#include "vulkan_allocator.h"
#include "vulkan_renderpass.h"

#include <vector>
#include <string>
#include <iostream>

ngv::Window::Window(const vk::Instance& instance, VulkanDevice& device, VulkanAllocator& allocator, uint32 graphicsQueueFamilyIndex, GLFWwindow* pWindow)
	: m_Device(device), m_Allocator(allocator)
{

	m_Instance = instance;
	m_GraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;

	glfwCreateWindowSurface(instance, pWindow,
		nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_Surface));

	

}

void ngv::Window::init()
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
	//fix depth stencil
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
	m_DynamicSemaphore = m_Device.device().createSemaphoreUnique(sci);

	typedef vk::CommandPoolCreateFlagBits ccbits;

	

}
