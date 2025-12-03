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
		~VulkanBuffer();
		VulkanBuffer(const VulkanBuffer& other) = delete;
		VulkanBuffer& operator=(const VulkanBuffer& other) = delete;
		VulkanBuffer(VulkanBuffer&& other) noexcept;
		VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;


		VkBuffer           buffer{ VK_NULL_HANDLE };
		VmaAllocation      allocation{ VK_NULL_HANDLE };
		VmaAllocationInfo  info{};          // keeps pMappedData, etc.
		VkDeviceSize       bufferSize{ 0 };
		VkBufferUsageFlags usageFlags{ 0 };
		VmaMemoryUsage     memoryUsage{ VMA_MEMORY_USAGE_UNKNOWN };
		VmaAllocator       allocator{ VK_NULL_HANDLE };

		bool IsValid() const { return buffer != VK_NULL_HANDLE; }

		static VulkanBuffer Create(
			VmaAllocator allocator,
			VkDeviceSize allocSize,
			VkBufferUsageFlags usage,
			VmaMemoryUsage memoryUsage,
			VmaAllocationCreateFlags allocFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT);

		void Destroy();
	};

	struct VulkanImage
	{
		VulkanImage();
		~VulkanImage();

		VulkanImage(VulkanImage&& other) noexcept;
		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;
		VulkanImage& operator=(VulkanImage&& other) noexcept;


		VkDevice vkDevice {VK_NULL_HANDLE};
		VmaAllocator allocator{ VK_NULL_HANDLE };
		VkImage image{ VK_NULL_HANDLE };
		VkImageView imageView{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		VkExtent3D imageExtent{ 0, 0, 1};
		VkFormat imageFormat {VK_FORMAT_UNDEFINED};
		/*
		VkSampler sampler {VK_NULL_HANDLE};
		VkDescriptorSet texture_descriptorSet{ VK_NULL_HANDLE };
		VkDescriptorSetLayout setlayout{ VK_NULL_HANDLE };
		*/
		VkQueue queue{ VK_NULL_HANDLE };

		static VulkanImage Create(
			Device& device,
			VkExtent3D size,
			VkFormat format, 
			VkImageUsageFlags usage, 
			bool mipmapped = false);

		static VulkanImage Create(
			void* data, 
			Device& device,
			VkExtent3D size,
			VkFormat format, 
			VkImageUsageFlags usage, 
			bool mipmapped = false);
		static VulkanImage Create();
		GNS_API void CreateSampler(VkFilter filter, VkSamplerAddressMode mode);
		void Destroy();

	private:
		void CreateImage(VkExtent3D size,
			VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		void CreateImage(void* data, VkExtent3D size,
			VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
	};

	struct VulkanTexture
	{
		VulkanImage image {};
		VkSampler sampler{ VK_NULL_HANDLE };
		VkFilter filter{ VK_FILTER_LINEAR };
		VkSamplerAddressMode samplerMode{ VK_SAMPLER_ADDRESS_MODE_REPEAT };

		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
		VkDescriptorSetLayout setLayout{ VK_NULL_HANDLE };

		VulkanTexture(Device& device, VkExtent3D size, VkFormat format, VkImageUsageFlags usage);
		VulkanTexture(void* data, Device& device, VkExtent3D size, VkFormat format, VkImageUsageFlags usage);
		VulkanTexture();
		~VulkanTexture();

		VulkanTexture(VulkanTexture&& other) = delete;
		VulkanTexture(const VulkanTexture&) = delete;
		VulkanTexture& operator=(const VulkanTexture&) = delete;
		VulkanTexture& operator=(VulkanTexture&& other) = delete;

		void Destroy();
		void CreateSampler(VkFilter filter, VkSamplerAddressMode mode);
		void CreateDefaultSampler();
	};

	struct VulkanMesh
	{
		VulkanBuffer indexBuffer{};
		VulkanBuffer vertexBuffer{};
		VkDeviceAddress vertexBufferAddress {0};

		VkIndexType indexType{VK_INDEX_TYPE_UINT32};
		VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

		struct IndexBufferRange {
			uint32_t startIndex{0};
			uint32_t count{0};
		} indexBufferRange = {};

		VulkanMesh() = default;
		~VulkanMesh() = default;

		VulkanMesh(VulkanMesh&& other) = delete;
		VulkanMesh(const VulkanMesh&) = delete;
		VulkanMesh& operator=(const VulkanMesh&) = delete;
		VulkanMesh& operator=(VulkanMesh&& other) = delete;

		void Destroy();
	};

	struct VulkanShader
	{
		friend class Device;
		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDevice device;
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


	struct PushConstants {
		glm::mat4 worldMatrix;
		uint64_t vertexBuffer;
	};

	enum class PassType
	{
		Graphics,
		Compute,
		Transfer
	};

	struct RenderPassContext
	{
		VkDevice device{ VK_NULL_HANDLE };
		VkCommandBuffer cmd { VK_NULL_HANDLE };
		VkPipeline pipeline { VK_NULL_HANDLE };
		VkPipelineBindPoint bindPoint { VK_PIPELINE_BIND_POINT_COMPUTE };
		VkShaderStageFlags shaderStages{ VK_SHADER_STAGE_ALL };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	};

	struct RenderPassData
	{
		bool writeDepth;
		bool writeColor;
		PassType passType;
		VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
		VulkanShader* shaderOverride {nullptr};
	};
	
	struct RenderPass
	{
		std::string name = "Invalid Pass";
		RenderPassData data {};
		RenderPassContext ctx {};
		std::function<void(RenderPass&)> record {nullptr};
	};
}
