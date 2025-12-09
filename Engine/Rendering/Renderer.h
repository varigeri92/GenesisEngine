#pragma once
#include <glm/glm.hpp>
#include "Vulkan/Device.h"
#include "../Object/Object.h"
#include "Objects/Material.h"
#include "Objects/Mesh.h"
#include "Objects/Camera.h"
#include "Objects/DrawObjects.h"

class Screen;

namespace gns::entity
{
	struct MeshComponent;
	struct Transform;
}

namespace gns
{
	class RenderSystem;
}

namespace gns::rendering
{
	class Shader;
}

namespace gns::rendering
{
	class Device;

	struct GlobalUniformData {
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewProj;
		glm::vec4 camPosition;
		glm::vec4 ambientColor; //w -> intensity
		glm::vec4 sunlightDirection; // w for sun power
		glm::vec4 sunlightColor;
		uint32_t pointLight_count;
		uint32_t spotLight_count;
		uint32_t dirLight_count;
		float exposure;
		float gamma;
	};


	class Renderer
	{
		friend class  gns::RenderSystem;
		friend class gns::rendering::Texture;

	public:
		Renderer(Screen* screen);
		~Renderer();
		Texture* GetDefaultTexture(const std::string& textureName);
		void DestroyTexture(TextureHandle handle);
		void DestroyMesh(MeshHandle handle);

	private:
		Screen* m_screen;
		GlobalUniformData globalUniform;

		std::vector<ObjectDrawData> objects;
		std::vector<size_t> objectIndices;
		std::vector<Mesh*> meshes;
		std::vector<Material*> materials;
		std::vector<Shader*> shaders;
		std::vector<DrawData> drawDataVector;

		std::vector<guid> m_shaderCache;

		void InitImGui();
		void BeginGuiFrame();
		void UpdateBuffers();
		void BuildDrawData();
		void CreatePipelineForShader(Shader* shader);
		Shader* CreateShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		Shader* ReCreateShader(const guid guid);
		void WaitForGPUIddle();

	public:
		void Draw();
		void UploadMesh(Mesh* mesh, uint32_t startIndex, uint32_t count) const;
		void CreateTextureDescriptorSet(Texture* texture);
		void UpdateTextureDescriptorSet(Texture* texture);

		VulkanBuffer CreateUniformBuffer(uint32_t size);
		VulkanBuffer CreateStagingBuffer(uint32_t size);
		VulkanBuffer CreateIndexBuffer(uint32_t size);
		VulkanBuffer CreateVertexBuffer(uint32_t size);

		TextureHandle CreateTexture(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage);
		TextureHandle CreateTexture(VkExtent3D size, VkFormat format, VkImageUsageFlags usage);
		VulkanTexture& GetTexture(TextureHandle handle);
		/*
		VulkanImage CreateImage(
			void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage);
		VulkanImage CreateImage(
			VkExtent3D size, VkFormat format, VkImageUsageFlags usage);
		 */
	};
}
