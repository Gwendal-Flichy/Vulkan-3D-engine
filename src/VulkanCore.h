#pragma once
#include <algorithm>
#include <array>
#include <assert.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_profiles.hpp>



#ifndef NDEBUG
# pragma comment(lib, "glfw3-s-d.lib")
#else /* NDEBUG */
# pragma comment(lib, "glfw3-s.lib")
#endif /* NDEBUG */

typedef void AssetManagerType;
#	define GLFW_INCLUDE_VULKAN
#	include <GLFW/glfw3.h>

// Define logging macros for Desktop
#	define LOGI(...)        \
		printf(__VA_ARGS__); \
		printf("\n")
#	define LOGW(...)        \
		printf(__VA_ARGS__); \
		printf("\n")
#	define LOGE(...)                 \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n")


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "RessourceManager.h"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
// Update paths to use glTF model and KTX2 texturev
const std::string MODEL_PATH = "models/cube_test_mesh_vulkan.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";
constexpr int     MAX_FRAMES_IN_FLIGHT = 2;
// Define the number of objects to render
constexpr int MAX_OBJECTS = 3;




struct AppInfo
{
	bool                profileSupported = false;
	VpProfileProperties profile;
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static vk::VertexInputBindingDescription getBindingDescription()
	{
		return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
	}

	static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		return {
			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
			vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)) };
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

template <>
struct std::hash<Vertex>
{
	size_t operator()(Vertex const& vertex) const noexcept
	{
		return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};



// Define a structure to hold per-object data
class Geometry
{
public:


	void setPosition(glm::vec3 newPosition)
	{
		position = newPosition;
	}
	void translate(glm::vec3 translation)
	{
		position += translation;
	}
	void setRotation(glm::vec3 newRotation)
	{
		rotation = newRotation;
	}
	void rotate(glm::vec3 rotation_)
	{
		rotation += rotation_;
	}
	void setScale(glm::vec3 newScale)
	{
		scale = newScale;
	}
	glm::vec3 getPosition()
	{
		return position;
	}
	glm::vec3 getRotation()
	{
		return rotation;
	}
	glm::vec3 getScale()
	{
		return scale;
	}
	const glm::vec3 getPosition() const
	{
		return position;
	}
	const glm::vec3 getRotation() const
	{
		return rotation;
	}
	const glm::vec3 getScale() const
	{
		return scale;
	}
	void setTexture(std::string texturePATH)
	{
		TexturePath = texturePATH;
	}
	void setModel(std::string ModelPATH)
	{
		ModelPath = ModelPATH;
	}

	void setTransformationMatrix(glm::mat4 newMatrix)
	{
		transformationMatrix = newMatrix;
	}
	glm::mat4 calculateTransformationMatrix()
	{
		transformationMatrix = glm::translate(transformationMatrix, position);
		transformationMatrix = glm::rotate(transformationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		transformationMatrix = glm::rotate(transformationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		transformationMatrix = glm::rotate(transformationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		transformationMatrix = glm::scale(transformationMatrix, scale);
		return transformationMatrix;
	}

	glm::mat4 getTransformationMatrix()
	{
		return transformationMatrix;
	}
	const glm::mat4 getTransformationMatrix() const
	{
		return transformationMatrix;
	}

	friend VulkanApplication;
protected:
	// Transform properties
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

	// Uniform buffer for this object (one per frame in flight)
	std::vector<vk::raii::Buffer>       uniformBuffers;
	std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
	std::vector<void*>                 uniformBuffersMapped;

	// Descriptor sets for this object (one per frame in flight)
	std::vector<vk::raii::DescriptorSet> descriptorSets;

	// Calculate model matrix based on position, rotation, and scale
	glm::mat4 getModelMatrix() const
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, scale);
		return model;
	}

	std::string TexturePath;
	std::string ModelPath;

	glm::mat4 transformationMatrix = glm::mat4(1.0f);
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class VulkanApplication
{
public:
	void run();
	
public:
	RessourceManager vulkanRessourceManage = RessourceManager(this);

	GLFWwindow* window = nullptr;

	AppInfo                          appInfo = {};
	vk::raii::Context                context;
	vk::raii::Instance               instance = nullptr;
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::SurfaceKHR             surface = nullptr;
	vk::raii::PhysicalDevice         physicalDevice = nullptr;
	vk::raii::Device                 device = nullptr;
	uint32_t                         queueIndex = ~0;
	vk::raii::Queue                  queue = nullptr;
	vk::raii::SwapchainKHR           swapChain = nullptr;
	std::vector<vk::Image>           swapChainImages;
	vk::SurfaceFormatKHR             swapChainSurfaceFormat;
	vk::Extent2D                     swapChainExtent;
	std::vector<vk::raii::ImageView> swapChainImageViews;

	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
	vk::raii::PipelineLayout      pipelineLayout = nullptr;
	vk::raii::Pipeline            graphicsPipeline = nullptr;

	vk::raii::Image        depthImage = nullptr;
	vk::raii::DeviceMemory depthImageMemory = nullptr;
	vk::raii::ImageView    depthImageView = nullptr;

	vk::raii::Sampler      textureSampler = nullptr;

	// Array of game objects to render
	std::array<Geometry, MAX_OBJECTS> gameObjects;

	vk::raii::DescriptorPool descriptorPool = nullptr;

	vk::raii::CommandPool                commandPool = nullptr;
	std::vector<vk::raii::CommandBuffer> commandBuffers;

	std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
	std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
	std::vector<vk::raii::Fence>     inFlightFences;
	uint32_t                         frameIndex = 0;

	bool framebufferResized = false;

	std::vector<const char*> requiredDeviceExtension = {
		vk::KHRSwapchainExtensionName,
		vk::KHRCreateRenderpass2ExtensionName };

	

	void initWindow();
	

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

public:
	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();
		createDepthResources();
		createTextureSampler();
		setupGameObjects();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}


	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);

	void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);

	void copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height);

	void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image, vk::raii::DeviceMemory& imageMemory);

	void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

	vk::raii::ImageView createImageView(vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags);

	void transition_image_layout(
		vk::Image               image,
		vk::ImageLayout         old_layout,
		vk::ImageLayout         new_layout,
		vk::AccessFlags2        src_access_mask,
		vk::AccessFlags2        dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask,
		vk::ImageAspectFlags    image_aspect_flags);

	void recreateSwapChain();
private:
	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			drawFrame();
		}

		device.waitIdle();
	}

	void cleanupSwapChain()
	{
		swapChainImageViews.clear();
		swapChain = nullptr;
	}

	void cleanup()
	{
		// Clean up resources in each GameObject
		for (auto& gameObject : gameObjects)
		{
			// Unmap memory
			for (size_t i = 0; i < gameObject.uniformBuffersMemory.size(); i++)
			{
				if (gameObject.uniformBuffersMapped[i] != nullptr)
				{
					gameObject.uniformBuffersMemory[i].unmapMemory();
				}
			}

			// Clear vectors to release resources
			gameObject.uniformBuffers.clear();
			gameObject.uniformBuffersMemory.clear();
			gameObject.uniformBuffersMapped.clear();
			vulkanRessourceManage.clear();
			gameObject.descriptorSets.clear();
		}

		// Clean up GLFW resources
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	


	void createInstance();
	

	void setupDebugMessenger();
	

	void createSurface();


	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();


	void createImageViews();


	void createDescriptorSetLayout();


	void createGraphicsPipeline();

	void createCommandPool();


	void createDepthResources();


	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

	vk::Format findDepthFormat() const;

	static bool hasStencilComponent(vk::Format format);


	void createTextureSampler();
	


	// Initialize the game objects with different positions, rotations, and scales
	void setupGameObjects()
	{
		// Object 1 - Center
		gameObjects[0].position = { 0.0f, 0.0f, 0.0f };
		gameObjects[0].rotation = { 0.0f, glm::radians(-90.0f), 0.0f };
		gameObjects[0].scale = { 1.0f, 1.0f, 1.0f };
		gameObjects[0].TexturePath = "textures/viking_room.png";
		gameObjects[0].ModelPath = "models/cube_test_mesh_vulkan.obj";

		// Object 2 - Left
		gameObjects[1].position = { -2.0f, 0.0f, -1.0f };
		gameObjects[1].rotation = { 0.0f, glm::radians(-45.0f), 0.0f };
		gameObjects[1].scale = { 0.75f, 0.75f, 0.75f };
		gameObjects[1].TexturePath = "textures/viking_room.png";
		gameObjects[1].ModelPath = "models/viking_room.obj";

		// Object 3 - Right
		gameObjects[2].position = { 2.0f, 0.0f, -1.0f };
		gameObjects[2].rotation = { 0.0f, glm::radians(45.0f), 0.0f };
		gameObjects[2].scale = { 0.75f, 0.75f, 0.75f };
		gameObjects[2].TexturePath = "textures/Sanstitre.png";
		gameObjects[2].ModelPath = "models/viking_room.obj";
	}

	// Create uniform buffers for each object
	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands();

	void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	void createCommandBuffers();

	void recordCommandBuffer(uint32_t imageIndex);

	

	void createSyncObjects();

	void updateUniformBuffers();

	void drawFrame();

	vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

	static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);

	static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

	static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

	std::vector<const char*> getRequiredInstanceExtensions() const;

	bool checkValidationLayerSupport() const;

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);

	std::vector<char> readFile(const std::string& filename);
};



