#pragma once
#include "../../../Utils/Logger.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace gns::rendering::utils
{
	//Helpers:
	VkCommandPoolCreateInfo CommandPoolCreateInfo(
		uint32_t queueFamilyIndex, 
		VkCommandPoolCreateFlags flags = 0);

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(
		VkCommandPool pool, 
		uint32_t count = 1, 
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkImageCreateInfo ImageCreateInfo(
		VkFormat format, 
		VkImageUsageFlags usageFlags, 
		VkExtent3D extent);

	VkImageViewCreateInfo ImageViewCreateInfo(
		VkFormat format, 
		VkImage image, 
		VkImageAspectFlags aspectFlags);

	VkDescriptorSetLayoutBinding DescriptorsetLayoutBinding(
		VkDescriptorType type, 
		VkShaderStageFlags stageFlags, 
		uint32_t binding);

	VkWriteDescriptorSet WriteDescriptorBuffer(
		VkDescriptorType type, 
		VkDescriptorSet dstSet, 
		VkDescriptorBufferInfo* bufferInfo, 
		uint32_t binding);

	VkCommandBufferBeginInfo CommandBufferBeginInfo(
		VkCommandBufferUsageFlags flags = 0);

	VkSubmitInfo SubmitInfo(
		VkCommandBuffer* cmd);

	VkSamplerCreateInfo SamplerCreateInfo(
		VkFilter filters, 
		VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

	VkWriteDescriptorSet WriteDescriptorImage(
		VkDescriptorType type, 
		VkDescriptorSet dstSet, 
		VkDescriptorImageInfo* imageInfo, 
		uint32_t binding);

	VkDescriptorBufferInfo CreateBufferInfo(
		VkBuffer buffer, 
		VkDeviceSize range, 
		VkDeviceSize offset = 0);

	VkDescriptorSetAllocateInfo CreateAllocateInfo(
		VkDescriptorPool pool, 
		const VkDescriptorSetLayout* setLayout, 
		uint32_t setCount = 1);

	VkDescriptorImageInfo CreateDescriptorImageInfo(
		VkSampler sampler, 
		VkImageView imageView, 
		VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags =0);

	VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

	void TransitionImage(
		VkCommandBuffer cmd, 
		VkImage image, 
		VkImageLayout currentLayout, 
		VkImageLayout newLayout);

	void CopyImageToImage(
		VkCommandBuffer cmd, 
		VkImage source, 
		VkImage destination, 
		VkExtent2D srcSize, 
		VkExtent2D dstSize,
		VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	);

	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask);

	VkRenderingAttachmentInfo AttachmentInfo(
		VkImageView view, 
		VkClearValue* clear, 
		VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


	//pipeline:
	VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader_module);

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo(VkPrimitiveTopology topology);

	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode polygon_mode);

	VkPipelineMultisampleStateCreateInfo MultisamplingStateCreateInfo();

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();

	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

	VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);


	VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);

	VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

	VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo,
	                         VkSemaphoreSubmitInfo* waitSemaphoreInfo);

	bool LoadShaderModule(const char* filePath,
		VkDevice device,
		VkShaderModule* outShaderModule);
	VkRenderingInfo RenderingInfo(
		VkExtent2D renderExtent,
		VkRenderingAttachmentInfo* colorAttachment,
		VkRenderingAttachmentInfo* depthAttachment);
	VkRenderingAttachmentInfo DepthAttachmentInfo(VkImageView image_view, VkImageLayout vk_image_layout);
}

#ifdef _DEBUG
#define VK_LOG
#endif

#ifdef VK_LOG

#define LOG_VK_VERBOSE(msg) \
	std::cout << AQUA <<"[VK_Verbose]: "<< DEFAULT << msg <<" "<<\
	GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_VK_INFO(msg) \
	std::cout << BLUE <<"[VK_Info]: "<< DEFAULT << msg <<" "<<\
	GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_VK_WARNING(msg) \
    std::cout << YELLOW << "[VK_Warning]: " << DEFAULT << msg <<" "<<\
    GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_VK_ERROR(msg) \
    std::cout << RED << "[VK_Error]: " << DEFAULT << msg <<" "<<\
    GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define _VK_CHECK(result, msg)\
    if(result != VK_SUCCESS)LOG_VK_ERROR(msg << " Result: " << result)

//-----------------
#else

#define LOG_VK_VERBOSE(msg)
#define LOG_VK_INFO(msg)
#define LOG_VK_WARNING(msg)
#define LOG_VK_ERROR(msg)
#define _VK_CHECK(result, msg) result

#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(  // NOLINT(clang-diagnostic-unused-function)
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		LOG_VK_VERBOSE(pCallbackData->pMessage);
	}

	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		LOG_VK_INFO(pCallbackData->pMessage);
	}

	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		LOG_VK_WARNING(pCallbackData->pMessage);
	}

	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		LOG_VK_ERROR(pCallbackData->pMessage);
	}

	return VK_FALSE;
}