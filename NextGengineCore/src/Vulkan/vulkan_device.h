#pragma once

#include "pch.h"

namespace ngv {

	class VulkanDevice {
	public:

		VulkanDevice() {}

		void setDevice(vk::UniqueDevice device);

		void setPhysicalDevice(vk::PhysicalDevice physicalDevice);

		const vk::Device device() const;

		const vk::PhysicalDevice physicalDevice() const;

		const vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties() const;

		const vk::PhysicalDeviceProperties physicalDeviceProperties() const;

		const vk::PhysicalDeviceLimits physicalDeviceLimits() const;

		const vk::PhysicalDeviceFeatures physicalDeviceFeatures() const;

		const vk::PhysicalDeviceFeatures enabledPhysicalDeviceFeatures() const;

		const std::vector<vk::QueueFamilyProperties>& queueFamilyProperties() const;

	private:
		vk::UniqueDevice m_Device;
		vk::PhysicalDevice m_PhysicalDevice;
		vk::PhysicalDeviceMemoryProperties m_MemProps;
		vk::PhysicalDeviceProperties m_Properties;
		vk::PhysicalDeviceFeatures m_Features;
		vk::PhysicalDeviceFeatures m_EnabledFeatures;
		std::vector<vk::QueueFamilyProperties> m_QueueProperties;
	};

	/// Factory for devices.
	class VulkanDeviceMaker {
	public:

		VulkanDeviceMaker() {};

		/// Set the default layers and extensions.
		VulkanDeviceMaker& setDefaultLayers();

		/// Add a layer. eg. "VK_LAYER_LUNARG_standard_validation"
		VulkanDeviceMaker& addLayer(const char* layer);

		/// Add an extension. eg. VK_EXT_DEBUG_REPORT_EXTENSION_NAME
		VulkanDeviceMaker& addExtension(const char* layer);

		/// Add one or more queues to the device from a certain family.
		VulkanDeviceMaker& addQueue(uint32 familyIndex, float priority = 0.0f, uint32 n = 1);

		/// Create a new logical device.
		vk::UniqueDevice createUnique(vk::PhysicalDevice physicalDevice);

	private:
		std::vector<const char*> m_Layers;
		std::vector<const char*> m_DeviceExtensions;
		std::vector<std::vector<float>> m_QueuePriorities;
		std::vector<vk::DeviceQueueCreateInfo> m_QueueCreateInfos;
		vk::ApplicationInfo m_AppInfo;

	};


}