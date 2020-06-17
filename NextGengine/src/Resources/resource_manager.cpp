#include "resource_manager.h"

#include "../Vulkan/vulkan_utility.h"
#include "../Vulkan/vulkan_device.h"
#include "../Vulkan/vulkan_storage.h"
#include "../Vulkan/vulkan_allocator.h"

ng::ResourceManager::ResourceManager(ngv::VulkanAllocator& allocator, ngv::VulkanDevice& device)
	: m_Allocator(allocator), m_Device(device)
{

}

std::shared_ptr<ngv::VertexBuffer> ng::ResourceManager::getVertexBuffer(std::string filename)
{
	return std::shared_ptr<ngv::VertexBuffer>();
}

std::shared_ptr<ngv::IndexBuffer> ng::ResourceManager::getIndexBuffer(std::string filename)
{
	return std::shared_ptr<ngv::IndexBuffer>();
}

std::shared_ptr<ngv::Texture2D> ng::ResourceManager::getTexture2D(std::string filename)
{
	return std::shared_ptr<ngv::Texture2D>();
}

std::unique_ptr<ng::Model2D> ng::ResourceManager::getModel2D(std::string filename)
{
#ifndef NDEBUG
	if (ng::getFileExtension(filename) != "model2d") {
		std::runtime_error("Tried to load Model2D with a file that isn't for Model2D's");
	}
#endif

	std::shared_ptr<ngv::VertexBuffer> vertexBuffer;
	std::shared_ptr<ngv::IndexBuffer> indexBuffer;


	// Load vertices and indices
	{
		struct Vertex2D {
			glm::vec2 vertex;
		};

		std::vector<Vertex2D> vertices;
		std::vector<uint32> indices;


		std::vector<uint8> bytes = ng::loadFile(filename + ".vertx");
#ifndef NDEBUG
		if (!(bytes.size() % 4)) {
			std::runtime_error("Incorrect vertices in " + filename + ".vertx");
		}
#endif
		vertices.resize(bytes.size() / 4);
		memcpy(vertices.data(), bytes.data(), bytes.size());
		vertexBuffer = VertexBuffer::make(m_Device, vertices.size() * sizeof(Vertex2D));
		m_Allocator.giveBufferAllocation(vertexBuffer);
		



		bytes = ng::loadFile(filename + ".indx");
#ifndef NDEBUG
		if (!(bytes.size() % 4)) {
			std::runtime_error("Incorrect indices in " + filename + ".indx");
		}
#endif
		indices.resize(bytes.size() / 4);
		memcpy(indices.data(), bytes.data(), bytes.size());
		indexBuffer = IndexBuffer::make(m_Device, indices.size() * sizeof(uint32));
		m_Allocator.giveBufferAllocation(indexBuffer);

	}








}
