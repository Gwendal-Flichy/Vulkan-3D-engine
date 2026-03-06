#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanApplication;

struct Vertex;

struct Model
{
	std::vector<Vertex>    vertices;
	std::vector<uint32_t>  indices;

	vk::raii::Buffer       vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexBufferMemory = nullptr;
	vk::raii::Buffer       indexBuffer = nullptr;
	vk::raii::DeviceMemory indexBufferMemory = nullptr;

	std::string modelPath;
};

struct Texture
{
	vk::raii::Image        textureImage = nullptr;
	vk::raii::DeviceMemory textureImageMemory = nullptr;
	vk::raii::ImageView    textureImageView = nullptr;
	std::string texturePath;
};



class RessourceManager
{
public:

	RessourceManager(VulkanApplication* VulkanApp);

	Texture& createTextureImageView(const std::string& TexturePATH);
	Model& loadModel(const std::string& modelPATH);
	std::vector<Texture>& getAllTexture() {return All_texture;}
	std::vector<Model>& getAllModels() { return All_models; }

	void clear();

private:
	std::vector<Texture> All_texture;
	std::vector<Model> All_models;
	vk::Format textureImageFormat = vk::Format::eR8G8B8A8Srgb;
	VulkanApplication& vulkan;
};

