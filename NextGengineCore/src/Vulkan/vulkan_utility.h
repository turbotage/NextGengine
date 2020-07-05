#pragma once

#include "pch.h"



namespace ngv {
	
	uint32 findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memprops, uint32 memoryTypeBits, vk::MemoryPropertyFlags seachFlags);

	uint32 mipScale(uint32 value, uint32 mipLevel);

	struct BlockParams {
		uint8 blockWidth;
		uint8 blockHeight;
		uint8 bytesPerBlock;
	};

	BlockParams getBlockParams(vk::Format format);

}

namespace ngv {

	/// Class for building shader modules and extracting metadata from shaders.
	class VulkanShaderModule {
	public:
		VulkanShaderModule() {}

		/// Construct a shader module from a file
		VulkanShaderModule(const vk::Device& device, const std::string& filename);

		/// Construct a shader module from a memory
		template<class InIter>
		VulkanShaderModule(const vk::Device& device, InIter begin, InIter end);

#ifdef NEXTGENGINE_SPIRV_SUPPORT
		/// A variable in a shader.
		struct Variable {
			// The name of the variable from the GLSL/HLSL
			std::string debugName;
			// The internal name (integer) of the variable
			int name;
			// The location in the binding.
			int location;
			// The binding in the descriptor set or I/O channel.
			int binding;
			// The descriptor set (for uniforms)
			int set;
			int instruction;
			// Storage class of the variable, eg. spv::StorageClass::Uniform
			spv::StorageClass storageClass;
		};

		/// Get a list of variables from the shader.
		/// 
		/// This exposes the Uniforms, inputs, outputs, push constants.
		/// See spv::StorageClass for more details.
		std::vector<Variable> getVariables() const;
#endif

		bool ok() const;
		vk::ShaderModule module();

		/// Write a C++ consumable dump of the shader.
		/// Todo: make this more idiomatic.
		std::ostream& write(std::ostream& os);

	private:

		struct State {
			std::vector<uint32> opcodes;
			vk::UniqueShaderModule module;
			bool ok = false;
		};

		State m_State;
	};

	template<class InIter>
	inline VulkanShaderModule::VulkanShaderModule(const vk::Device& device, InIter begin, InIter end)
	{
		m_State.opcodes.assign(begin, end);
		vk::ShaderModuleCreateInfo createInfo;
		createInfo.codeSize = m_State.opcodes.size() * 4;
		createInfo.pCode = m_State.opcodes.data();
		m_State.module = device.createShaderModuleUnique(createInfo);
		m_State.ok = true;
	}


}

namespace ngv {

	void setImageLayout(vk::CommandBuffer cb, vk::Image image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange, 
		vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands);
 

	void setImageLayout(vk::CommandBuffer cb, vk::Image image, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout,
		vk::ImageLayout newImageLayout, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask);

}