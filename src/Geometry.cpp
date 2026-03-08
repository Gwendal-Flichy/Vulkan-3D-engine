#include "VulkanCore.h"


void Geometry::createUniformBuffers()
{
	this->uniformBuffers.clear();
	this->uniformBuffersMemory.clear();
	this->uniformBuffersMapped.clear();

	// Create uniform buffers for each frame in flight
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DeviceSize         bufferSize = sizeof(UniformBufferObject);
		vk::raii::Buffer       buffer({});
		vk::raii::DeviceMemory bufferMem({});
		renderer.createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMem);
		this->uniformBuffers.emplace_back(std::move(buffer));
		this->uniformBuffersMemory.emplace_back(std::move(bufferMem));
		this->uniformBuffersMapped.emplace_back(this->uniformBuffersMemory[i].mapMemory(0, bufferSize));
	}
}


void Geometry::createDescriptorPool()
{
	// We need MAX_OBJECTS * MAX_FRAMES_IN_FLIGHT descriptor sets
	size_t GeometryNumber = renderer.AllGeometries.size();
	std::array poolSize{
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer,  MAX_FRAMES_IN_FLIGHT),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT) };
	vk::DescriptorPoolCreateInfo poolInfo{
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
		.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
		.pPoolSizes = poolSize.data() };
	descriptorPool = vk::raii::DescriptorPool(renderer.device, poolInfo);
}


void Geometry::createDescriptorSets()
{
	// Create descriptor sets for each frame in flight
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *renderer.descriptorSetLayout);
	vk::DescriptorSetAllocateInfo        allocInfo{
			   .descriptorPool = *descriptorPool,
			   .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
			   .pSetLayouts = layouts.data() };

	this->descriptorSets.clear();
	this->descriptorSets = renderer.device.allocateDescriptorSets(allocInfo);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo{
			.buffer = *this->uniformBuffers[i],
			.offset = 0,
			.range = sizeof(UniformBufferObject) };
		vk::DescriptorImageInfo imageInfo{
			.sampler = *renderer.textureSampler,
			.imageView = *renderer.vulkanRessourceManage.createTextureImageView(this->TexturePath).textureImageView,
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };
		std::array descriptorWrites{
			vk::WriteDescriptorSet{
				.dstSet = *this->descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.pBufferInfo = &bufferInfo},
			vk::WriteDescriptorSet{
				.dstSet = *this->descriptorSets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo} };
		renderer.device.updateDescriptorSets(descriptorWrites, {});
	}

}