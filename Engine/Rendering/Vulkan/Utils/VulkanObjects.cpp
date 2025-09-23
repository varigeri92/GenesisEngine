#include "gnspch.h"
#include <vma/vk_mem_alloc.h>
#include "VulkanObjects.h"
#include "../Device.h"

gns::rendering::VulkanBuffer::VulkanBuffer() :  buffer(VK_NULL_HANDLE), allocation(VK_NULL_HANDLE), info() {}

gns::rendering::VulkanBuffer::~VulkanBuffer()
{
	LOG_INFO("Delete Buffer");
	if (destroyed) return;
	Destroy();
}

gns::rendering::VulkanBuffer* gns::rendering::VulkanBuffer::CreateBuffer(
	size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VulkanBuffer* buffer = new VulkanBuffer();

	buffer->bufferSize = allocSize;
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;


	VmaAllocationCreateInfo vmaAllocInfo = {};
	vmaAllocInfo.usage = memoryUsage;
	vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	_VK_CHECK(vmaCreateBuffer(Device::sAllocator, &bufferInfo, &vmaAllocInfo,
		&buffer->buffer,
		&buffer->allocation,
		&buffer->info), "Bufferallocation Failed");

	return buffer;
}

void gns::rendering::VulkanBuffer::Destroy()
{
	destroyed = true;
	vmaDestroyBuffer(Device::sAllocator, buffer, allocation);
}


// IMAGE:
gns::rendering::VulkanImage::VulkanImage() : destroyed(true) {}

gns::rendering::VulkanImage::~VulkanImage()
{
	Destroy();
}

void gns::rendering::VulkanImage::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
	destroyed = false;
	imageFormat = format;
	imageExtent = size;

	VkImageCreateInfo img_info = utils::ImageCreateInfo(format, usage, size);
	if (mipmapped)
	{
		img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo allocinfo = {};
	allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// allocate and create the image
	_VK_CHECK(vmaCreateImage(Device::sAllocator, &img_info, &allocinfo, &image, &allocation, nullptr),
		"Failed to create Image");

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	// build a image-view for the image
	VkImageViewCreateInfo view_info = utils::ImageViewCreateInfo(format, image, aspectFlag);
	view_info.subresourceRange.levelCount = img_info.mipLevels;

	_VK_CHECK(vkCreateImageView(Device::sDevice, &view_info, nullptr, &imageView), "Failed To createImageView");
}

void gns::rendering::VulkanImage::CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
                                              bool mipmapped)
{

	const size_t data_size = size.depth * size.width * size.height * 4;
	VulkanBuffer* uploadBuffer = VulkanBuffer::CreateBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	memcpy(uploadBuffer->info.pMappedData, data, data_size);

	CreateImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

	Device::ImmediateSubmit([&](VkCommandBuffer cmd) {

		utils::TransitionImage(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = size;

		vkCmdCopyBufferToImage(cmd, uploadBuffer->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
			&copyRegion);

		utils::TransitionImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});
	delete(uploadBuffer);
}

void gns::rendering::VulkanImage::CreateSampler(VkFilter filter, VkSamplerAddressMode mode)
{
	VkSamplerCreateInfo samplerInfo = utils::SamplerCreateInfo(filter, mode);
	vkCreateSampler(Device::sDevice, &samplerInfo, nullptr, &sampler);
	hasSampler = true;
}

void gns::rendering::VulkanImage::Destroy()
{
	if(destroyed)
		return;
	vkDestroyImageView(Device::sDevice, imageView, nullptr);
	vmaDestroyImage(Device::sAllocator, image, allocation);

	if (hasSampler)
		vkDestroySampler(Device::sDevice, sampler, nullptr);

	destroyed = true;
}

void gns::rendering::VulkanShader::Destroy()
{
	vkDestroyPipelineLayout(Device::sDevice, m_pipelineLayout, nullptr);
	vkDestroyPipeline(Device::sDevice, m_pipeline, nullptr);
	vkDestroyDescriptorSetLayout(Device::sDevice, m_descriptorSetLayout, nullptr);
}
