#pragma once

#include "vulkandef.h"

namespace ngv {

	/// Factory for renderpasses.
	/// example:
	///     RenderpassMaker rpm;
	///     rpm.subpassBegin(vk::PipelineBindPoint::eGraphics);
	///     rpm.subpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal);
	///    
	///     rpm.attachmentDescription(attachmentDesc);
	///     rpm.subpassDependency(dependency);
	///     s.renderPass_ = rpm.createUnique(device);
	class VulkanRenderpassMaker {
	public:

		VulkanRenderpassMaker() {};

		/// Begin an attachment description.
		/// After this you can call setAttachment* many times
		void beginAttachment(vk::Format format);

		void setAttachmentFlags(vk::AttachmentDescriptionFlags value);
		void setAttachmentFormat(vk::Format value);
		void setAttachmentSamples(vk::SampleCountFlagBits value);
		void setAttachmentLoadOp(vk::AttachmentLoadOp value);
		void setAttachmentStoreOp(vk::AttachmentStoreOp value);
		void setAttachmentStencilLoadOp(vk::AttachmentLoadOp value);
		void setAttachmentStencilStoreOp(vk::AttachmentStoreOp value);
		void setAttachmentInitialLayout(vk::ImageLayout value);
		void setAttachmentFinalLayout(vk::ImageLayout value);

		/// Start a subpass description.
		/// After this you can can call addSubpassColorAttachment many times
		/// and setSubpassDepthStencilAttachment once.
		void beginSubpass(vk::PipelineBindPoint bp);

		void addSubpassColorAttachment(vk::ImageLayout layout, uint32_t attachment);

		void setSubpassDepthStencilAttachment(vk::ImageLayout layout, uint32_t attachment);

		vk::UniqueRenderPass createUnique(const vk::Device& device) const;

		void beginDependency(uint32_t srcSubpass, uint32_t dstSubpass);

		void setDependencySrcSubpass(uint32_t value);
		void setDependencyDstSubpass(uint32_t value);
		void setDependencySrcStageMask(vk::PipelineStageFlags value);
		void setDependencyDstStageMask(vk::PipelineStageFlags value);
		void setDependencySrcAccessMask(vk::AccessFlags value);
		void setDependencyDstAccessMask(vk::AccessFlags value);
		void setDependencyDependencyFlags(vk::DependencyFlags value);

	private:

		constexpr static int m_MaxRefs = 64;

		vk::AttachmentReference* getAttachmentReference();

		struct State {
			std::vector<vk::AttachmentDescription> attachmentDescriptions;
			std::vector<vk::SubpassDescription> subpassDescriptions;
			std::vector<vk::SubpassDependency> subpassDependencies;
			std::array<vk::AttachmentReference, m_MaxRefs> attachmentReferences;
			int numRefs = 0;
			bool ok = false;
		};
		State m_State;

	};



}