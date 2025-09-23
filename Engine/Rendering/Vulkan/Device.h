﻿#pragma once
#include <vulkan/vulkan.h>
#include <deque>
#include <functional>
#include <span>
#include "Utils/vkutils.h"
#include "VkBootstrap.h"
#include "Utils/VulkanObjects.h"
#include "Utils/VkDescriptors.h"

class Screen;

namespace gns::rendering
{
	struct Mesh;
	struct Texture;
}

namespace gns::rendering
{
	struct ObjectDrawData;
	struct Material;
	class Shader;
	class PipelineBuilder;
}

namespace gns::rendering
{
	class Renderer;
}

namespace gns
{
	namespace entity
	{
		struct MeshComponent;
	}

	class RenderSystem;
}

namespace gns::rendering
{
	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void Push(std::function<void()>&& function);

		void Flush();
	};

	struct FrameData {
		VkSemaphore presentSemaphore;
		VkSemaphore renderSemaphore;
		VkFence renderFence;
		VkCommandPool commandPool;
		VkCommandBuffer mainCommandBuffer;
		DeletionQueue deletionQueue;
		DescriptorAllocatorGrowable frameDescriptors;
	};

	struct ComputePushConstants {
		glm::vec4 data1;
		glm::vec4 data2;
		glm::vec4 data3;
		glm::vec4 data4;
	};

	struct ComputeEffect {
		const char* name;
		VkPipeline pipeline;
		VkPipelineLayout layout;
		ComputePushConstants data;
	};

	struct ImmeduateSubmitStruct
	{
		// immediate submit structures
		VkFence _immFence;
		VkCommandBuffer _immCommandBuffer;
		VkCommandPool _immCommandPool;
	};

	class Device
	{
		friend class gns::RenderSystem;
		friend class gns::rendering::PipelineBuilder;
		friend struct gns::rendering::VulkanBuffer;
		friend struct gns::rendering::VulkanImage;
		friend class gns::rendering::Renderer;
		friend class gns::rendering::VulkanShader;
		friend class gns::rendering::Texture;
	public:
		Device(Screen* screen);
		~Device();
		Device(Device& other) = delete;
		Device operator=(Device& other) = delete;

		VulkanImage _whiteImage;
		VulkanImage _blackImage;
		VulkanImage _blueImage;

	private:
		Screen* m_screen;
		// Move this for sure:
		std::vector<ComputeEffect> backgroundEffects;
		int currentBackgroundEffect{ 0 };
		VkExtent2D drawExtent;
		VkPipelineLayout m_gradientPipelineLayout; //default compute pipeline layout.

		VkDescriptorSetLayout m_perFrameDescriptorLayout;
		VulkanBuffer* m_objectStorageBuffer;
		VulkanBuffer* m_pointLightStorageBuffer;
		VulkanBuffer* m_spotLightStorageBuffer;
		VulkanBuffer* m_gpuSceneDataBuffer;

		Texture* offscreen_Texture;

		// ------------------

		vkb::Instance m_vkb_instance;
		vkb::Device m_vkb_device;

		static VkDevice sDevice;
		static VmaAllocator sAllocator;
		uint32_t m_imageCount;
		uint32_t m_imageIndex;
		VkInstance m_instance;
		VkSurfaceKHR m_surface;

		VkPhysicalDevice m_physicalDevice;
		VkPhysicalDeviceProperties m_gpuProperties;
		VkPhysicalDeviceMemoryProperties memoryProperties;

		DeletionQueue m_deletionQueue;

		static VkQueue sGraphicsQueue;
		uint32_t m_graphicsFamilyIndex;
		VkQueue m_transferQueue;
		uint32_t m_transferFamilyIndex;
		VkQueue m_presentQueue;
		uint32_t m_presentFamilyIndex;
		VkQueue m_computeQueue;
		uint32_t m_computeFamilyIndex;

		vkb::Swapchain m_vkb_swapchain;
		VkExtent2D m_swapchainExtent;

		VkSwapchainKHR m_swapchain;
		VkFormat m_swapchainFormat;


		std::vector<VkImageView> m_imageViews;
		std::vector<VkImage> m_images;
		std::vector<FrameData> m_frames;

		VulkanImage m_renderTargetImage;
		VulkanImage m_depthImage;

		DescriptorAllocator m_globalDescriptorAllocator;
		VkDescriptorSet m_renderTargetDescriptor;
		VkDescriptorSetLayout m_renderTargetSetLayout;

		static ImmeduateSubmitStruct sImmediateSubmitStruct;
		Shader* m_currentBoundShader = nullptr;
		Material* m_currentMaterial = nullptr;

		size_t m_currentBoundShader_guid = 0;
		size_t m_currentBoundMaterial_guid = 0;

		void CreateImageDataFromScratch();

		void InitPipelines();
		void InitBackgroundPipelines();

		bool InitVulkan();
		void CreateSwapchain();
		void ResizeSwapchain();
		void DestroySwapchain();
		void CreatePipeline(Shader& shader);

		void InitCommands();
		void InitDescriptors();
		void InitSyncStructures();
		void Draw(
			std::vector<ObjectDrawData>& objects,
			std::vector<size_t>& indices, 
			std::vector<Mesh*>& meshes,
			std::vector<Material*>& materials
		);

		bool BeginDraw(uint32_t &swapchainImageIndex);
		void StartFrame(VkCommandBuffer& cmd);
		void EndFrame(VkCommandBuffer& cmd, uint32_t& swapchainImageIndex);
		void EndDraw();
		void DrawBackground(VkCommandBuffer cmd);
		void SetDrawStructs(VkCommandBuffer cmd);
		void UpdatePerFrameDescriptors(VkDescriptorSet& perFrameSet);
		void UpdateMaterialData(VkDescriptorSet& imageSet, Material& material);

		void DrawGeometry(VkCommandBuffer cmd, std::vector<ObjectDrawData>& objects, 
			std::vector<size_t>& indices, 
			std::vector<rendering::Mesh*>& meshes,
			std::vector<Material*>& materials);

		void UpdateUniformBuffer(void* data, const VulkanBuffer* buffer);
		void UpdateGlobalUbo(void* data, size_t size);
		void UpdateStorageBuffer(void* data, size_t size);
		void UpdatePointLightBuffer(void* data, size_t size);
		void UpdateSpotLightBuffer(void* data, size_t size);

		static void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
		void DrawImGui(VkCommandBuffer cmd, VkImageView targetImageView);

		void UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices, 
			VulkanMesh& vulkanMesh);

		FrameData& GetCurrentFrame();

		uint32_t GetMemoryType(uint32_t typeBits,
			VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
		
	};
}
