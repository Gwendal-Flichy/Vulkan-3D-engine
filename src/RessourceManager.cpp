#include "RessourceManager.h"
#include "VulkanCore.h"

#include "stb_image.h"
#include "tiny_obj_loader.h"

RessourceManager::RessourceManager(VulkanApplication* VulkanApp) : vulkan(*VulkanApp)
{
}

Model& RessourceManager::loadModel(const std::string& modelPATH)
{
	for (auto& model : All_models)
	{
		if (model.modelPath == modelPATH)
		{
			return model;
		}
	}

	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPATH.c_str()))
	{
		throw std::runtime_error(warn + err);
	}

	Model model;
	model.modelPath = modelPATH;

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2] };

			vertex.texCoord = { 0.0f, 0.0f };
			if (index.texcoord_index >= 0)
			{
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };
			}

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (!uniqueVertices.contains(vertex))
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(model.vertices.size());
				model.vertices.push_back(vertex);
			}

			model.indices.push_back(uniqueVertices[vertex]);
		}
	}

	{
		vk::DeviceSize         bufferSize = sizeof(model.vertices[0]) * model.vertices.size();
		vk::raii::Buffer       stagingBuffer({});
		vk::raii::DeviceMemory stagingBufferMemory({});
		vulkan.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void* data = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(data, model.vertices.data(), bufferSize);
		stagingBufferMemory.unmapMemory();

		vulkan.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, model.vertexBuffer, model.vertexBufferMemory);

		vulkan.copyBuffer(stagingBuffer, model.vertexBuffer, bufferSize);
	}

	{
		vk::DeviceSize         bufferSize = sizeof(model.indices[0]) * model.indices.size();
		vk::raii::Buffer       stagingBuffer({});
		vk::raii::DeviceMemory stagingBufferMemory({});
		vulkan.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void* data = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(data, model.indices.data(), bufferSize);
		stagingBufferMemory.unmapMemory();

		vulkan.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, model.indexBuffer, model.indexBufferMemory);

		vulkan.copyBuffer(stagingBuffer, model.indexBuffer, bufferSize);
	}

	All_models.push_back(std::move(model));
	return All_models.back();
}



Texture& RessourceManager::createTextureImageView(const std::string& TexturePATH)
{
	for (auto& texture : All_texture)
	{
		if (texture.texturePath == TexturePATH)
		{
			return texture;
		}
	}

	
	Texture test;
	test.texturePath = TexturePATH;
	int            texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(TexturePATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image!");
	}

	vk::raii::Buffer       stagingBuffer({});
	vk::raii::DeviceMemory stagingBufferMemory({});
	vulkan.createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

	void* data = stagingBufferMemory.mapMemory(0, imageSize);
	memcpy(data, pixels, imageSize);
	stagingBufferMemory.unmapMemory();

	stbi_image_free(pixels);

	vulkan.createImage(texWidth, texHeight, textureImageFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, test.textureImage, test.textureImageMemory);

	vulkan.transitionImageLayout(test.textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	vulkan.copyBufferToImage(stagingBuffer, test.textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	vulkan.transitionImageLayout(test.textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	test.textureImageView = vulkan.createImageView(test.textureImage, textureImageFormat, vk::ImageAspectFlagBits::eColor);
	All_texture.push_back(std::move(test));
	return All_texture.back();
}



void RessourceManager::clear()
{
	for (auto& model : All_models)
	{
		if (model.vertexBufferMemory != nullptr)
			model.vertexBufferMemory.unmapMemory();
		if (model.indexBufferMemory != nullptr)
			model.indexBufferMemory.unmapMemory();
		model.vertexBuffer.clear();
		model.vertexBufferMemory.clear();
		model.indexBuffer.clear();
		model.indexBufferMemory.clear();
	}

	for (auto& texture : All_texture)
	{

		if (texture.textureImageMemory != nullptr)
			texture.textureImageMemory.unmapMemory();
		texture.textureImage.clear();
		texture.textureImageMemory.clear();
		texture.textureImageView.clear();
	}
	All_texture.clear();
}
