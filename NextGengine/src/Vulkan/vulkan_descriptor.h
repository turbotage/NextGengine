#pragma once
#include "vulkandef.h"


namespace ngv {
	
	class VulkanDevice;

	class VulkanDescriptorSetLayoutMaker {
	public:

		VulkanDescriptorSetLayoutMaker() {}

		void buffer(uint32 binding, vk::DescriptorType descriptorType, 
			vk::ShaderStageFlags stageFlags, uint32 descriptorCount);

		void image(uint32 binding, vk::DescriptorType descriptorType,
			vk::ShaderStageFlags stageFlags, uint32 descriptorCount);

		void samplers(uint32 binding, vk::DescriptorType descriptorType,
			vk::ShaderStageFlags stageFlags, const std::vector<vk::Sampler> immutableSamplers);

		void bufferView(uint32 binding, vk::DescriptorType descriptorType,
			vk::ShaderStageFlags stageFlags, uint32 descriptorCount);

		vk::UniqueDescriptorSetLayout createUnique(VulkanDevice& device) const;

	private:
		std::vector<vk::DescriptorSetLayoutBinding> m_Bindings;
		std::vector<std::vector<vk::Sampler>> m_Samplers;
		int numSamplers = 0;
	};






	class VulkanDescriptorSetMaker {
	public:

		VulkanDescriptorSetMaker() {}

		void layout(vk::DescriptorSetLayout layout);

		/// Allocate a vector of non-self-deleting descriptor sets
		/// Note: descriptor sets get freed with the pool
		std::vector<vk::DescriptorSet> create(VulkanDevice& device, vk::DescriptorPool descriptorPool) const;

		/// Allocate a vector of self-deleting descriptor sets
		std::vector<vk::UniqueDescriptorSet> createUnique(VulkanDevice& device, vk::DescriptorPool descriptorPool) const;

	private:
		
		std::vector<vk::DescriptorSetLayout> m_Layouts;

	};






	class VulkanDescriptorSetUpdater {
	public:

		VulkanDescriptorSetUpdater(int maxBuffers = 10, int maxImages = 10, int maxBufferViews = 0);

		/// Call this to begin a new descriptor set.
		void beginDescriptorSet(vk::DescriptorSet dstSet);

		/// Call this to begin a new set of images.
		void beginImages(uint32 binding, uint32 dstArrayElement, vk::DescriptorType descriptorType);

		/// Call this to add a combined image sampler.
		void image(vk::Sampler sampler, vk::ImageView imageView, vk::ImageLayout imageLayout);

		/// Call this to start defining buffers.
		void beginBuffers(uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType);
		
		/// Call this to add a new buffer.
		void buffer(vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range);

		/// Call this to start adding buffer views. (for example, writable images).
		void beginBufferViews(uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType);

		/// Call this to add a buffer view. (Texel images)
		void bufferView(vk::BufferView view);

		/// Copy an existing descriptor.
		void copy(vk::DescriptorSet srcSet, uint32_t srcBinding, uint32_t srcArrayElement,
			vk::DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount);

		void update(const vk::Device& device) const;

		bool ok() const;

	private:

		std::vector<vk::DescriptorBufferInfo> m_BufferInfos;
		std::vector<vk::DescriptorImageInfo> m_ImageInfos;
		std::vector<vk::WriteDescriptorSet> m_DescriptorWrites;
		std::vector<vk::CopyDescriptorSet> m_DescriptorCopies;
		std::vector<vk::BufferView> m_BufferViews;
		vk::DescriptorSet m_DescSet;
		int m_NumBuffers = 0;
		int m_NumImages = 0;
		int m_NumBufferViews = 0;
		bool m_Ok = true;

	};


}

