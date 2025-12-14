#include "gnspch.h"
#include <vma/vk_mem_alloc.h>
#include "VulkanObjects.h"

#include <algorithm>

#include "../Device.h"

gns::rendering::VulkanBuffer::VulkanBuffer()
	: buffer(VK_NULL_HANDLE)
	, allocation(VK_NULL_HANDLE)
	, info{}
	, bufferSize(0)
	, usageFlags(0)
	, memoryUsage(VMA_MEMORY_USAGE_UNKNOWN)
	, allocator(VK_NULL_HANDLE)
{
}

gns::rendering::VulkanBuffer::~VulkanBuffer()
{
	Destroy();
}

gns::rendering::VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
{
	*this = std::move(other);
}

gns::rendering::VulkanBuffer& gns::rendering::VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
{
	if (this != &other)
	{
		Destroy();

		buffer = other.buffer;
		allocation = other.allocation;
		info = other.info;
		bufferSize = other.bufferSize;
		usageFlags = other.usageFlags;
		memoryUsage = other.memoryUsage;
		allocator = other.allocator;

		other.buffer = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
		other.allocator = VK_NULL_HANDLE;
		other.bufferSize = 0;
		other.usageFlags = 0;
		other.memoryUsage = VMA_MEMORY_USAGE_UNKNOWN;
		other.info = {};
	}

	return *this;
}


gns::rendering::VulkanBuffer gns::rendering::VulkanBuffer::Create(VmaAllocator allocator, VkDeviceSize allocSize,
	VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags)
{
	VulkanBuffer buffer{};

	buffer.bufferSize = allocSize;
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;


	VmaAllocationCreateInfo vmaAllocInfo = {};
	vmaAllocInfo.usage = memoryUsage;
	vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	_VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocInfo,
		&buffer.buffer,
		&buffer.allocation,
		&buffer.info), "Bufferallocation Failed");

	return buffer;
}

void gns::rendering::VulkanBuffer::Destroy()
{
	if (!buffer || !allocation || !allocator)
		return;

	vmaDestroyBuffer(allocator, buffer, allocation);

	buffer = VK_NULL_HANDLE;
	allocation = VK_NULL_HANDLE;
	allocator = VK_NULL_HANDLE;
	bufferSize = 0;
	usageFlags = 0;
	memoryUsage = VMA_MEMORY_USAGE_UNKNOWN;
	info = {};
}


// IMAGE:
gns::rendering::VulkanImage::VulkanImage()
{
	image = VK_NULL_HANDLE;
	imageView = VK_NULL_HANDLE;
	allocation = VK_NULL_HANDLE;
	imageExtent = { 0,0,1 };
	imageFormat = VK_FORMAT_UNDEFINED;
}

gns::rendering::VulkanImage::~VulkanImage()
{
	Destroy();
}

gns::rendering::VulkanImage::VulkanImage(gns::rendering::VulkanImage&& other) noexcept
{
	*this = std::move(other);
}

gns::rendering::VulkanImage& gns::rendering::VulkanImage::operator=(VulkanImage&& other) noexcept
{
	if (this != &other)
	{
		queue = other.queue;
		vkDevice = other.vkDevice;
		allocator = other.allocator;
		image = other.image;
		imageView = other.imageView;
		allocation = other.allocation;
		imageExtent = other.imageExtent;
		imageFormat = other.imageFormat;
		
		other.queue = VK_NULL_HANDLE;
		other.vkDevice = VK_NULL_HANDLE;
		other.allocator = VK_NULL_HANDLE;
		other.image = VK_NULL_HANDLE;
		other.imageView = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
		other.imageExtent = {};
		other.imageFormat = VK_FORMAT_UNDEFINED;
	}
	return *this;
}

gns::rendering::VulkanImage gns::rendering::VulkanImage::Create(Device& device, VkExtent3D size, VkFormat format,
                                                                VkImageUsageFlags usage, bool mipmapped)
{
	VulkanImage image{};
	image.vkDevice = device.GetDevice();
	image.allocator = device.GetAllocator();
	image.queue = device.GetGraphicsQueue();
	image.CreateImage(size, format, usage, mipmapped);
	return image;
}

gns::rendering::VulkanImage gns::rendering::VulkanImage::Create(void* data, Device& device, VkExtent3D size, VkFormat format,
	VkImageUsageFlags usage, bool mipmapped)
{
	VulkanImage image{};
	image.vkDevice = device.GetDevice();
	image.allocator = device.GetAllocator();
	image.queue = device.GetGraphicsQueue();
	image.CreateImage(data, size, format, usage, mipmapped);
	return image;
}

gns::rendering::VulkanImage gns::rendering::VulkanImage::Create()
{
	return {};
}

void gns::rendering::VulkanImage::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
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
	_VK_CHECK(vmaCreateImage(allocator, &img_info, &allocinfo, &image, &allocation, nullptr),
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

	_VK_CHECK(vkCreateImageView(vkDevice, &view_info, nullptr, &imageView), "Failed To createImageView");
}

void gns::rendering::VulkanImage::CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
                                              bool mipmapped)
{
	const size_t data_size = size.depth * size.width * size.height * 4;
	VulkanBuffer uploadBuffer = VulkanBuffer::Create(allocator, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	memcpy(uploadBuffer.info.pMappedData, data, data_size);

	CreateImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

	Device::ImmediateSubmit(vkDevice, queue, [&](VkCommandBuffer cmd) {

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

		vkCmdCopyBufferToImage(cmd, uploadBuffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
			&copyRegion);

		utils::TransitionImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});
}


void gns::rendering::VulkanImage::Destroy()
{
	LOG_INFO("Destroying Vulkan Image!");
	if(imageView != VK_NULL_HANDLE)
		vkDestroyImageView(vkDevice, imageView, nullptr);

	if(image != VK_NULL_HANDLE)
		vmaDestroyImage(allocator, image, allocation);

	image = VK_NULL_HANDLE;
	imageView = VK_NULL_HANDLE;
	allocation = VK_NULL_HANDLE;
	imageExtent = { 0,0,1 };
	imageFormat = VK_FORMAT_UNDEFINED;

}


gns::rendering::VulkanTexture::VulkanTexture(Device& device, VkExtent3D size, VkFormat format, VkImageUsageFlags usage)
{
	image = VulkanImage::Create(device, size, format, usage);
	CreateDefaultSampler();
}

gns::rendering::VulkanTexture::VulkanTexture(void* data, Device& device, VkExtent3D size, VkFormat format, VkImageUsageFlags usage)
{
	image = VulkanImage::Create(data, device, size, format, usage);
	CreateDefaultSampler();
}

gns::rendering::VulkanTexture::VulkanTexture()
{
	LOG_INFO("VulkanTexture Default constuctor called!");
}

gns::rendering::VulkanTexture::~VulkanTexture()
{
	image.Destroy();
}

void gns::rendering::VulkanTexture::CreateSampler(VkFilter filter, VkSamplerAddressMode mode)
{
	filterMode = filter;
	samplerMode = mode;
	VkSamplerCreateInfo samplerInfo = utils::SamplerCreateInfo(filter, mode);
	vkCreateSampler(image.vkDevice, &samplerInfo, nullptr, &sampler);
}

void gns::rendering::VulkanTexture::CreateDefaultSampler()
{
	CreateSampler(filterMode, samplerMode);
}

void gns::rendering::VulkanTexture::Destroy()
{

	if (sampler != VK_NULL_HANDLE)
		vkDestroySampler(image.vkDevice, sampler, nullptr);

	sampler = VK_NULL_HANDLE;
	descriptorSet = VK_NULL_HANDLE;
	setLayout = VK_NULL_HANDLE;
	image.Destroy();
}

void gns::rendering::VulkanMesh::Destroy()
{
	vertexBuffer.Destroy();
	indexBuffer.Destroy();
}

void gns::rendering::VulkanShader::Destroy()
{
	vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
	vkDestroyPipeline(device, m_pipeline, nullptr);
	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
}

