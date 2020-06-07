#include "vulkan_window.h"

#include "vulkandef.h"

#include <vector>
#include <string>
#include <iostream>

ngv::Window::Window(const vk::Instance& instance, const vk::Device& device, const vk::PhysicalDevice& physicalDevice, uint32 graphicsQueueFamilyIndex, uint32 width, uint32 height, const char* title)
{
	m_Width = width;
	m_Height = height;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

	vk::SurfaceKHR surface;
	glfwCreateWindowSurface(instance, m_pWindow,
		nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface));

	

}

void ngv::Window::init(const vk::Instance& instance, const vk::Device& device, const vk::PhysicalDevice& physicalDevice, uint32 graphicsQueueFamilyIndex, vk::SurfaceKHR surface)
{
	m_Surface = surface;
	m_Instance = instance;
	m_Device = device;
	
	m_PresentQueueFamily = 0;
	auto& pd = physicalDevice;
	auto qprops = pd.getQueueFamilyProperties();
	bool found = false;
	for (uint32 qi = 0; qi != qprops.size(); ++qi) {
		auto& qprop = qprops[qi];
		if (pd.getSurfaceSupportKHR(qi, m_Surface) && 
			(qprop.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics) {
			m_PresentQueueFamily = qi;
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
	std::array<uint32_t, 2> queueFamilyIndices = { graphicsQueueFamilyIndex, m_PresentQueueFamily };
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
	m_Swapchain = device.createSwapchainKHRUnique(swapinfo);

	m_SwapChainImages = m_Device.getSwapchainImagesKHR(*m_Swapchain);
	for (auto& img : m_SwapChainImages) {
		vk::ImageViewCreateInfo ci{};
		ci.image = img;
		ci.viewType = vk::ImageViewType::e2D;
		ci.format = m_SwapChainImageFormat;
		m_SwapChainViews.emplace_back(m_Device.createImageView(ci));
	}

	auto memprops = physicalDevice.getMemoryProperties();
	//fix depth stencil

}
