#pragma once
#include "vkutils.h"
#include "glm/glm.hpp"

namespace gns::rendering
{
	class Device;

	struct VulkanBuffer
	{

	private:
		bool destroyed = false;
	public:

		VulkanBuffer();
		VulkanBuffer(const VulkanBuffer& other) = delete;
		VulkanBuffer(VulkanBuffer&& other) = delete;
		VulkanBuffer& operator=(const VulkanBuffer& other) = delete;
		VulkanBuffer& operator=(VulkanBuffer&& other) = delete;

		~VulkanBuffer();

		void Destroy();

		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo info;
		size_t bufferSize;

		static VulkanBuffer* CreateBuffer(size_t allocSize, 
		VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	};

	struct VulkanImage
	{
	private:
		bool destroyed = false;
		bool hasSampler = false;
	public:
		VulkanImage();
		~VulkanImage();

		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VkExtent3D imageExtent;
		VkFormat imageFormat;

		VkSampler sampler;
		VkDescriptorSet texture_descriptorSet;
		VkDescriptorSetLayout setlayout;

		void CreateImage(VkExtent3D size,
			VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		void CreateImage(void* data, VkExtent3D size,
			VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		GNS_API void CreateSampler(VkFilter filter, VkSamplerAddressMode mode);

		void Destroy();
	};

	struct VulkanShader
	{
		friend class Device;
		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		VkDescriptorSetLayout m_descriptorSetLayout;
		void Destroy();
	};

	struct Vertex
	{
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
		glm::vec4 tangent;
		glm::vec4 biTangent;
	};

	struct VulkanMesh
	{
		VulkanBuffer* indexBuffer;
		VulkanBuffer* vertexBuffer;
		VkDeviceAddress vertexBufferAddress;
	};

	struct PushConstants {
		glm::mat4 worldMatrix;
		uint64_t vertexBuffer;
	};

	struct RenderPass
	{
		VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
		VkPipeline pipelineOverride = VK_NULL_HANDLE;
	};

}
