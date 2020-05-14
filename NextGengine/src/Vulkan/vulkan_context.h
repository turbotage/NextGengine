#pragma once

#include "vulkandef.h"

#include <set>
#include <functional>
#include <list>
#include <queue>

namespace ngv {


	struct VulkanDeviceCreateInfo : public vk::DeviceCreateInfo {
		std::vector<vk::DeviceQueueCreateInfo> deviceQueues;
		std::vector<std::vector<float>> deviceQueuesPriorities;

		void addQueueFamily(uint32 queueFamilyIndex, vk::ArrayProxy<float> priorities);

		void addQueueFamily(uint32 queueFamilyIndex, size_t count = 1);

		void update();

	};

	using StringList = std::list<std::string>;
	using CStringVector = std::vector<const char*>;

	using DevicePickerFunction = std::function<vk::PhysicalDevice(const std::vector<vk::PhysicalDevice>&)>;
	using DeviceFeaturesPickerFunction = std::function<void(const vk::PhysicalDevice&, vk::PhysicalDeviceFeatures2&)>;
	using DeviceExtensionsPickerFunction = std::function<std::set<std::string>(const vk::PhysicalDevice&)>;
	using InstanceExtensionsPickerFunction = std::function<std::set<std::string>()>;
	using InstanceExtensionsPickerFunctions = std::list<InstanceExtensionsPickerFunction>;

	using SemaphoreStagePair = std::pair<const vk::Semaphore, const vk::PipelineStageFlags>;

	using LayerVector = std::vector<const char*>;

	using VoidLambda = std::function<void()>;
	using VoidLambdaList = std::list<VoidLambda>;
	using FencedLambda = std::pair<vk::Fence, VoidLambda>;
	using FencedLambdaQueue = std::queue<FencedLambda>;

	class VulkanContext {
	public:
		
		static std::set<std::string> getAvailableLayers();

		static std::vector<vk::ExtensionProperties> getExtensions();

		static std::set<std::string> getExtensionNames();

		static bool isExtensionPresent(const std::string& extensionName);

		static std::vector<vk::ExtensionProperties> getDeviceExtensions(const vk::PhysicalDevice& physicalDevice);

		static std::set<std::string> getDeviceExtensionNames(const vk::PhysicalDevice& physicalDevice);

		static bool isDeviceExtensionPresent(const vk::PhysicalDevice& physicalDevice, const std::string& extension);

		void requireExtensions(const vk::ArrayProxy<const std::string>& requestedExtensions);

		void requireDeviceExtensions(const vk::ArrayProxy<const std::string>& requestedExtensions);

		void addInstanceExtensionPicker(const InstanceExtensionsPickerFunction& function);

		void setDevicePicker(const DevicePickerFunction& picker);

		void setDeviceFeaturesPicker(const DeviceFeaturesPickerFunction& picker);

		void setDeviceExtensionsPicker(const DeviceExtensionsPickerFunction& picker);

		void setValidationEnabled(bool enable);

		void createInstance(uint32_t version = VK_MAKE_VERSION(1, 1, 0));

		void createDevice(const vk::SurfaceKHR& surface = nullptr);

		void destroy();

		uint32_t findQueue(const vk::QueueFlags& desiredFlags, const vk::SurfaceKHR& presentSurface = nullptr) const;

		template <typename T>
		void trash(T value) const {
			trash<T>(value, [](T t) { t.destroy(); });
		}

		template <typename T>
		void trash(T value, std::function<void(T t)> destructor) const {
			if (!value) {
				return;
			}
			dumpster.push_back([=] { destructor(value); });
		}

		template <typename T>
		void trashAll(std::vector<T>& values, std::function<void(const std::vector<T>& t)> destructor) const {
			if (values.empty()) {
				return;
			}
			dumpster.push_back([=] { destructor(values); });
			// Clear the buffer
			values.clear();
		}

		void trashPipeline(vk::Pipeline& pipeline) const;

		void trashCommandBuffers(const vk::CommandPool& commandPool, std::vector<vk::CommandBuffer>& cmdBuffers) const;

		// Should be called from time to time by the application to migrate zombie resources
		// to the recycler along with a fence that will be signalled when the objects are
		// safe to delete.
		void emptyDumpster(vk::Fence fence);

		// Check the recycler fences for signalled status.  Any that are signalled will have their corresponding
		// lambdas executed, freeing up the associated resources
		void recycle();

		// Create an image memory barrier for changing the layout of
		// an image and put it into an active command buffer
		// See chapter 11.4 "vk::Image Layout" for details

		void setImageLayout(vk::CommandBuffer cmdbuffer,
			vk::Image image,
			vk::ImageLayout oldImageLayout,
			vk::ImageLayout newImageLayout,
			vk::ImageSubresourceRange subresourceRange) const;

		// Fixed sub resource on first mip level and layer
		void setImageLayout(vk::CommandBuffer cmdbuffer,
			vk::Image image,
			vk::ImageLayout oldImageLayout,
			vk::ImageLayout newImageLayout) const;

		// Fixed sub resource on first mip level and layer
		void setImageLayout(vk::CommandBuffer cmdbuffer,
			vk::Image image,
			vk::ImageAspectFlags aspectMask,
			vk::ImageLayout oldImageLayout,
			vk::ImageLayout newImageLayout) const;

		void setImageLayout(vk::Image image, 
			vk::ImageLayout oldImageLayout, 
			vk::ImageLayout newImageLayout, 
			vk::ImageSubresourceRange subresourceRange) const;

		// Fixed sub resource on first mip level and layer
		void setImageLayout(vk::Image image, 
			vk::ImageAspectFlags aspectMask, 
			vk::ImageLayout oldImageLayout, 
			vk::ImageLayout newImageLayout) const;

		vk::CommandPool getCommandPool() const;

		void destroyCommandPool() const;

		std::vector<vk::CommandBuffer> allocateCommandBuffers(uint32_t count, 
			vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;

		vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;

		void flushCommandBuffer(vk::CommandBuffer& commandBuffer) const;

		// Create a short lived command buffer which is immediately executed and released
		// This function is intended for initialization only.  It incurs a queue and device
		// flush and may impact performance if used in non-setup code
		void withPrimaryCommandBuffer(const std::function<void(const vk::CommandBuffer& commandBuffer)>& f) const;

		void copyToMemory(const vk::DeviceMemory& memory, const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0) const;

		template <typename T>
		void copyToMemory(const vk::DeviceMemory& memory, const T& data, size_t offset = 0) const {
			copyToMemory(memory, &data, sizeof(T), offset);
		}

		template <typename T>
		void copyToMemory(const vk::DeviceMemory& memory, const std::vector<T>& data, size_t offset = 0) const {
			copyToMemory(memory, data.data(), data.size() * sizeof(T), offset);
		}

		vk::Bool32 getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties, uint32_t* typeIndex) const;

		uint32_t getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties) const;

		void submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers,
			const vk::ArrayProxy<const vk::Semaphore>& wait = {},
			const vk::ArrayProxy<const vk::PipelineStageFlags>& waitStages = {},
			const vk::ArrayProxy<const vk::Semaphore>& signals = {},
			const vk::Fence& fence = vk::Fence()) const;

		void submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers,
			const vk::ArrayProxy<const SemaphoreStagePair>& wait = {},
			const vk::ArrayProxy<const vk::Semaphore>& signals = {},
			const vk::Fence& fence = vk::Fence()) const;

		// Helper submit function when there is only one wait semaphore, to remove ambiguity
		void submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers,
			const SemaphoreStagePair& wait,
			const vk::ArrayProxy<const vk::Semaphore>& signals = {},
			const vk::Fence& fence = vk::Fence()) const;

		vk::Format getSupportedDepthFormat() const;

	protected:

		void pickDevice(const vk::SurfaceKHR& surface);

		void buildDevice();

	private:

		static CStringVector toCStrings(const StringList& values);

		static CStringVector toCStrings(const vk::ArrayProxy<const std::string>& values);

		static CStringVector filterLayers(const StringList& desiredLayers);

	public:

		// Vulkan instance, stores all per-application states
		vk::Instance instance;
		std::vector<vk::PhysicalDevice> physicalDevices;
		// Physical device (GPU) that Vulkan will ise
		vk::PhysicalDevice physicalDevice;

		// Queue family properties
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
		// Stores physical device properties (for e.g. checking device limits)
		vk::PhysicalDeviceProperties deviceProperties;
		// Stores phyiscal device features (for e.g. checking if a feature is available)
		vk::PhysicalDeviceFeatures deviceFeatures;

		vk::PhysicalDeviceFeatures2 enabledFeatures2;
		vk::PhysicalDeviceFeatures& enabledFeatures = enabledFeatures2.features;
		// Stores all available memory (type) properties for the physical device
		vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;
		// Logical device, application's view of the physical device (GPU)
		vk::Device device;
		// vk::Pipeline cache object
		vk::PipelineCache pipelineCache;
		// Helper for accessing functionality not available in the statically linked Vulkan library
		vk::DispatchLoaderDynamic dynamicDispatch;

		struct QueueIndices {
			uint32_t graphics{ VK_QUEUE_FAMILY_IGNORED };
			uint32_t transfer{ VK_QUEUE_FAMILY_IGNORED };
			uint32_t compute{ VK_QUEUE_FAMILY_IGNORED };
		} queueIndices;

		vk::Queue queue;

	protected:

		// A collection of items queued for destruction.  Once a fence has been created
		// for a queued submit, these items can be moved to the recycler for actual destruction
		// by calling the rec

		mutable VoidLambdaList dumpster;
		FencedLambdaQueue recycler;

		InstanceExtensionsPickerFunctions instanceExtensionsPickers;

#ifndef NDEBUG
		bool enableValidation = true;
#else
		bool enableValidation = false;
#endif

		bool enableDebugMarkers = false;

	private:
		std::set<std::string> m_RequiredExtensions;
		std::set<std::string> m_RequiredDeviceExtensions;

		DevicePickerFunction devicePicker = [](const std::vector<vk::PhysicalDevice>& devices) -> vk::PhysicalDevice { return devices[0]; };
		DeviceFeaturesPickerFunction deviceFeaturesPicker = [](const vk::PhysicalDevice& device, vk::PhysicalDeviceFeatures2& features) {};
		DeviceExtensionsPickerFunction deviceExtensionsPicker = [](const vk::PhysicalDevice& device) -> std::set<std::string> { return {}; };

#ifdef WIN32
		static __declspec(thread) vk::CommandPool m_CmdPool;
#else
		static thread_local vk::CommandPool m_CmdPool;
#endif


	};

}