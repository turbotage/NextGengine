#pragma once

#include "vulkandef.h"

namespace ngv {

	//Some forward declarations
	class VulkanShaderModule;



	/// A class for building pipeline layouts.
	/// Pipeline layouts describe the descriptor sets and push constants used by the shaders.
	class VulkanPipelineLayoutMaker {
	public:
		VulkanPipelineLayoutMaker() {}

		vk::UniquePipelineLayout createUnique(const vk::Device& device) const;

		void addDescriptorSetLayout(vk::DescriptorSetLayout layout);

		void addPushConstantRange(vk::ShaderStageFlags stageFlags, uint32 offset, uint32 size);

	private:
		std::vector<vk::DescriptorSetLayout> m_SetLayouts;
		std::vector<vk::PushConstantRange> m_PushConstantRanges;
	};







	/// A class for building pipelines.
	/// All the state of the pipeline is exposed through individual calls.
	/// The pipeline encapsulates all the OpenGL state in a single object.
	/// This includes vertex buffer layouts, blend operations, shaders, line width etc.
	/// This class exposes all the values as individuals so a pipeline can be customised.
	/// The default is to generate a working pipeline.
	class VulkanPipelineMaker {
	public:

		VulkanPipelineMaker(uint32 width, uint32 height);

		vk::UniquePipeline createUnique(const vk::Device& device,
			const vk::PipelineCache& pipelineCache,
			const vk::PipelineLayout& pipelineLayout,
			const vk::RenderPass& renderPass, bool defaultBlend = true);

		/// Add a shader module to the pipeline.
		void addShader(vk::ShaderStageFlagBits stage, VulkanShaderModule& shader, const char* entryPoint = "main");

		/// Add a blend state to the pipeline for one colour attachment.
		/// If you don't do this, a default is used.
		void addColorBlend(const vk::PipelineColorBlendAttachmentState& state);

		void setSubpass(uint32 subpass);

		/// Begin setting colour blend value
		/// If you don't do this, a default is used.
		/// Follow this with blendEnable() blendSrcColorBlendFactor() etc.
		/// Default is a regular alpha blend.
		void blendBegin(vk::Bool32 enable);
		/// Enable or disable blending (called after blendBegin())
		void blendEnable(vk::Bool32 value);
		/// Source colour blend factor (called after blendBegin())
		void blendSrcColorBlendFactor(vk::BlendFactor value);
		/// Destination colour blend factor (called after blendBegin())
		void blendDstColorBlendFactor(vk::BlendFactor value);
		/// Blend operation (called after blendBegin())
		void blendColorBlendOp(vk::BlendOp value);
		/// Source alpha blend factor (called after blendBegin())
		void blendSrcAlphaBlendFactor(vk::BlendFactor value);
		/// Destination alpha blend factor (called after blendBegin())
		void blendDstAlphaBlendFactor(vk::BlendFactor value);
		/// Alpha operation (called after blendBegin())
		void blendAlphaBlendOp(vk::BlendOp value);
		/// Colour write mask (called after blendBegin())
		void blendColorWriteMask(vk::ColorComponentFlags value);


		/// Add a vertex attribute to the pipeline.
		void addVertexAttribute(uint32_t location, uint32_t binding, vk::Format format, uint32_t offset);
		/// Add a vertex attribute to the pipeline.
		void addVertexAttribute(const vk::VertexInputAttributeDescription& desc);
		/// Add a vertex binding to the pipeline.
		/// Usually only one of these is needed to specify the stride.
		/// Vertices can also be delivered one per instance.
		void addVertexBinding(uint32_t binding, uint32_t stride, vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);
		/// Add a vertex binding to the pipeline.
		/// Usually only one of these is needed to specify the stride.
		/// Vertices can also be delivered one per instance.
		void addVertexBinding(const vk::VertexInputBindingDescription& desc);


		/// Specify the topology of the pipeline.
		/// Usually this is a triangle list, but points and lines are possible too.
		VulkanPipelineMaker& topology(vk::PrimitiveTopology topology);
		/// Enable or disable primitive restart.
		/// If using triangle strips, for example, this allows a special index value (0xffff or 0xffffffff) to start a new strip.
		VulkanPipelineMaker& primitiveRestartEnable(vk::Bool32 primitiveRestartEnable);
		/// Set a whole new input assembly state.
		/// Note you can set individual values with their own call
		VulkanPipelineMaker& inputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo& value);
		/// Set the viewport value.
		/// Usually there is only one viewport, but you can have multiple viewports active for rendering cubemaps or VR stereo pair
		VulkanPipelineMaker& viewport(const vk::Viewport& value);
		/// Set the scissor value.
		/// This defines the area that the fragment shaders can write to. For example, if you are rendering a portal or a mirror.
		VulkanPipelineMaker& scissor(const vk::Rect2D& value);


		/// Set a whole rasterization state.
		/// Note you can set individual values with their own call
		VulkanPipelineMaker& rasterizationState(const vk::PipelineRasterizationStateCreateInfo& value);
		VulkanPipelineMaker& depthClampEnable(vk::Bool32 value);
		VulkanPipelineMaker& rasterizerDiscardEnable(vk::Bool32 value);
		VulkanPipelineMaker& polygonMode(vk::PolygonMode value);
		VulkanPipelineMaker& cullMode(vk::CullModeFlags value);
		VulkanPipelineMaker& frontFace(vk::FrontFace value);
		VulkanPipelineMaker& depthBiasEnable(vk::Bool32 value);
		VulkanPipelineMaker& depthBiasConstantFactor(float value);
		VulkanPipelineMaker& depthBiasClamp(float value);
		VulkanPipelineMaker& depthBiasSlopeFactor(float value);
		VulkanPipelineMaker& lineWidth(float value);


		/// Set a whole multi sample state.
		/// Note you can set individual values with their own call
		VulkanPipelineMaker& multisampleState(const vk::PipelineMultisampleStateCreateInfo& value);
		VulkanPipelineMaker& rasterizationSamples(vk::SampleCountFlagBits value);
		VulkanPipelineMaker& sampleShadingEnable(vk::Bool32 value);
		VulkanPipelineMaker& minSampleShading(float value);
		VulkanPipelineMaker& pSampleMask(const vk::SampleMask* value);
		VulkanPipelineMaker& alphaToCoverageEnable(vk::Bool32 value);
		VulkanPipelineMaker& alphaToOneEnable(vk::Bool32 value);

		/// Set a whole depth stencil state.
		/// Note you can set individual values with their own call
		VulkanPipelineMaker& depthStencilState(const vk::PipelineDepthStencilStateCreateInfo& value);
		VulkanPipelineMaker& depthTestEnable(vk::Bool32 value);
		VulkanPipelineMaker& depthWriteEnable(vk::Bool32 value);
		VulkanPipelineMaker& depthCompareOp(vk::CompareOp value);
		VulkanPipelineMaker& depthBoundsTestEnable(vk::Bool32 value);
		VulkanPipelineMaker& stencilTestEnable(vk::Bool32 value);
		VulkanPipelineMaker& front(vk::StencilOpState value);
		VulkanPipelineMaker& back(vk::StencilOpState value);
		VulkanPipelineMaker& minDepthBounds(float value);
		VulkanPipelineMaker& maxDepthBounds(float value);

		/// Set a whole colour blend state.
		/// Note you can set individual values with their own call
		VulkanPipelineMaker& colorBlendState(const vk::PipelineColorBlendStateCreateInfo& value);
		VulkanPipelineMaker& logicOpEnable(vk::Bool32 value);
		VulkanPipelineMaker& logicOp(vk::LogicOp value);
		VulkanPipelineMaker& blendConstants(float r, float g, float b, float a);

		VulkanPipelineMaker& dynamicState(vk::DynamicState value);

	private:
		vk::Viewport m_Viewport;
		vk::Rect2D m_Scissor;

		vk::PipelineInputAssemblyStateCreateInfo m_InputAssemblyState;
		vk::PipelineRasterizationStateCreateInfo m_RasterizationState;
		vk::PipelineMultisampleStateCreateInfo m_MultisampleState;
		vk::PipelineDepthStencilStateCreateInfo m_DepthStencilState;
		vk::PipelineColorBlendStateCreateInfo m_ColorBlendState;
		std::vector<vk::PipelineColorBlendAttachmentState> m_ColorBlendAttachments;
		std::vector<vk::PipelineShaderStageCreateInfo> m_Modules;
		std::vector<vk::VertexInputAttributeDescription> m_VertexAttributeDescriptions;
		std::vector<vk::VertexInputBindingDescription> m_VertexBindingDescriptions;
		std::vector<vk::DynamicState> m_DynamicState;
		uint32 m_Subpass = 0;

	};








	/// Builds compute pipelines
	class VulkanComputePipelineMaker {
	public:

		VulkanComputePipelineMaker() {}

		void shader(vk::ShaderStageFlagBits stage, ngv::VulkanShaderModule& shader, const char* entryPoint = "main");

		VulkanComputePipelineMaker& module(const vk::PipelineShaderStageCreateInfo& value);

		vk::UniquePipeline createUnique(vk::Device device, const vk::PipelineCache& pipelineCache, const vk::PipelineLayout& pipelineLayout);

	private:
		vk::PipelineShaderStageCreateInfo m_StageInfo;

	};







}