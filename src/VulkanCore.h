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

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};


class Camera
{
public:

	void processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
		xOffset *= mouseSensitivity;
		yOffset *= mouseSensitivity;

		yaw += xOffset;
		pitch += yOffset;

		// Constrain pitch to avoid flipping
		if (constrainPitch) {
			pitch = std::clamp(pitch, -89.0f, 89.0f);
		}

		// Update camera vectors based on updated Euler angles
		updateCameraVectors();
	}

	void updateCameraVectors() 
	{
		// Calculate the new front vector
		glm::vec3 newFront;
		newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		newFront.y = sin(glm::radians(pitch));
		newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(newFront);

		// Recalculate the right and up vectors
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}

	glm::mat4 getViewMatrix() const {
		return glm::lookAt(position, position + front, up);
	}

	glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const {
		return glm::perspective(glm::radians(zoom), aspectRatio, nearPlane, farPlane);
	}

	// Spatial positioning and orientation vectors
// These form the camera's local coordinate system in world space
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);     // Camera's location in world coordinates
	glm::vec3 front = glm::vec3(0.0f, 1.0f, 0.0f);        // Forward direction (where camera is looking)
	glm::vec3 up;           // Camera's local up direction (for roll control)
	glm::vec3 right;        // Camera's local right direction (perpendicular to front and up)
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);      // Global up vector reference (typically Y-axis)
	// Rotation representation using Euler angles
// Provides intuitive control while managing gimbal lock and other rotation complexities
	float yaw = -90.0f;              // Horizontal rotation around the world up-axis (left-right looking)
	float pitch = 0.0f;            // Vertical rotation around the camera's right axis (up-down looking)

	float mouseSensitivity = 1.f; // Multiplier for mouse input to rotation angle conversion
	float zoom = 45.f;             // Field of view control for perspective projection

};




// Define a structure to hold per-object data
class Geometry
{
public:
	Geometry(VulkanApplication& Renderer) : renderer(Renderer)
	{
	}

	
	void destroy()
	{
		
		// Unmap memory
		for (size_t i = 0; i < this->uniformBuffersMemory.size(); i++)
		{
			if (this->uniformBuffersMapped[i] != nullptr)
			{
				this->uniformBuffersMemory[i].unmapMemory();
			}
		}

		this->uniformBuffers.clear();
		this->uniformBuffersMemory.clear();
		this->uniformBuffersMapped.clear();
		
		this->descriptorSets.clear();
		this->descriptorPool.clear();
	}

	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();


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
	VulkanApplication& renderer;
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
	vk::raii::DescriptorPool descriptorPool = nullptr;

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

	std::string TexturePath = "textures/Sanstitre.png";;
	std::string ModelPath = "models/cube_test_mesh_vulkan.obj";

	glm::mat4 transformationMatrix = glm::mat4(1.0f);
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
	//std::array<Geometry, MAX_OBJECTS> gameObjects;
	std::vector<Geometry> AllGeometries;

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


	Geometry& createGeometry()
	{
		Geometry geometry(*this);
		AllGeometries.push_back(std::move(geometry));
		AllGeometries.back().createUniformBuffers();
		AllGeometries.back().createDescriptorPool();
		AllGeometries.back().createDescriptorSets();
		return AllGeometries.back();
	}


	void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
		// State persistence for calculating movement deltas
		// Static variables maintain state between callback invocations
		static bool firstMouse = true;          // Flag to handle initial mouse position
		static float lastX = 0.0f, lastY = 0.0f;  // Previous mouse position for delta calculation

		// Handle initial mouse position to prevent sudden camera jumps
		// First callback provides absolute position, not relative movement
		if (firstMouse) {
			lastX = xpos;               // Initialize previous position
			lastY = ypos;
			firstMouse = false;         // Disable special handling for subsequent calls
		}

		// Calculate mouse movement deltas since last callback
		// These deltas represent the amount and direction of mouse movement
		float xoffset = xpos - lastX;                   // Horizontal movement (left-right)
		float yoffset = lastY - ypos;                   // Vertical movement (inverted: screen Y increases downward, camera pitch increases upward)

		// Update state for next callback iteration
		lastX = xpos;
		lastY = ypos;

		// Convert mouse movement to camera rotation
		// Delta values drive continuous camera orientation changes
		camera.processMouseMovement(xoffset, yoffset,true);
	}

	int count = 0;

	Camera camera;
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
		for (auto& gameObject : AllGeometries)
		{
			gameObject.destroy();
			vulkanRessourceManage.clear();
			
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
		//Object 1 - Center
		Geometry geo1(*this);
		geo1.position = { 0.0f, 0.0f, 0.0f };
		geo1.rotation = { 0.0f, glm::radians(-90.0f), 0.0f };
		geo1.scale = { 1.0f, 1.0f, 1.0f };
		geo1.TexturePath = "textures/viking_room.png";
		geo1.ModelPath = "models/cube_test_mesh_vulkan.obj";
		AllGeometries.push_back(std::move(geo1));
		AllGeometries.back().createUniformBuffers();
		AllGeometries.back().createDescriptorPool();
		AllGeometries.back().createDescriptorSets();

		// Object 2 - Left
		Geometry geo2(*this);
		geo2.position = { -2.0f, 0.0f, -1.0f };
		geo2.rotation = { 0.0f, glm::radians(-45.0f), 0.0f };
		geo2.scale = { 0.75f, 0.75f, 0.75f };
		geo2.TexturePath = "textures/viking_room.png";
		geo2.ModelPath = "models/viking_room.obj";
		AllGeometries.push_back(std::move(geo2));
		AllGeometries.back().createUniformBuffers();
		AllGeometries.back().createDescriptorPool();
		AllGeometries.back().createDescriptorSets();

		// Object 3 - Right
		Geometry geo3(*this);
		geo3.position = { 2.0f, 0.0f, -1.0f };
		geo3.rotation = { 0.0f, glm::radians(45.0f), 0.0f };
		geo3.scale = { 0.75f, 0.75f, 0.75f };
		geo3.TexturePath = "textures/Sanstitre.png";
		geo3.ModelPath = "models/viking_room.obj";
		AllGeometries.push_back(std::move(geo3));
		AllGeometries.back().createUniformBuffers();
		AllGeometries.back().createDescriptorPool();
		AllGeometries.back().createDescriptorSets();
	}


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



