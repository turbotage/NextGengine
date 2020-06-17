#pragma once


class ngv::VulkanAllocator;
class ngv::VulkanDevice;
class ngv::VertexBuffer;
class ngv::IndexBuffer;
class ngv::Texture2D;

namespace ng {

	class Model2D;



	class ResourceManager {
	public:

		ResourceManager(ngv::VulkanAllocator& allocator, ngv::VulkanDevice& device);

		std::shared_ptr<ngv::VertexBuffer> getVertexBuffer(std::string filename);

		std::shared_ptr<ngv::IndexBuffer> getIndexBuffer(std::string filename);

		std::shared_ptr<ngv::Texture2D> getTexture2D(std::string filename);

		std::unique_ptr<ng::Model2D> getModel2D(std::string filename);

	private:

		<

	private:

		ngv::VulkanAllocator& m_Allocator;
		ngv::VulkanDevice& m_Device;

	};

}