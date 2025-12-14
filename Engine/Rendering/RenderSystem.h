#pragma once
#include "imgui.h"
#include "../ECS/Component.h"
#include "glm/glm.hpp"

#include "../ECS/SystemBase.h"
#include "Objects/Texture.h"
#include "Vulkan/PipelineBuilder.h"

class Screen;

namespace gns
{
	namespace entity
	{
		struct Transform;
	}

	namespace rendering
	{
		struct Camera;
		class Renderer;

		struct LightingSettings
		{
			uint32_t shadowMapSize{ 1024 };
			uint32_t pcf_kernelSize{ 1 };
			float normalOffset{ 0.00005f };
			float shadowBias{ 0.00550f };
			float slopeScale{ 0.00100f};
			float halfExtent{ 15.f };
			float nearPlane{ -100.f };
		};
	}




	class RenderSystem final : public gns::SystemBase
	{
		friend class gns::rendering::Texture;
	private:
		gns::rendering::Renderer* m_renderer;
		rendering::Camera* m_camera;
		entity::Transform* m_cameraTransform;
		Screen* m_renderScreen;
		rendering::Texture* m_offScreenRenderTargetTexture;
		rendering::LightingSettings m_lightingSettings;
		void UpdateCamera();
		void BeginGuiFrame();
		void EndGuiFrame();

	public:
		RenderSystem(Screen* screen);

		GNS_API rendering::Texture* GetRenderTargetTexture();
		GNS_API ImTextureID GetRenderTargetTextureID();
		GNS_API Screen* GetTargetScren();
		GNS_API void SetTargetScreenSize(uint32_t width, uint32_t height);

		GNS_API void SetActiveCamera(rendering::Camera* camera, entity::Transform* transform);

		GNS_API rendering::Shader* CreateShader(const std::string& name,const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		GNS_API rendering::Shader* ReCreateShader(const guid guid);
		GNS_API rendering::Shader* GetShader(guid guid);
		GNS_API rendering::Shader* GetShader(const std::string& name);

		GNS_API rendering::Texture* CreateTexture(const std::string& texturePath);
		GNS_API void CreateTextureDescriptor(rendering::Texture* texture);
		GNS_API void UpdateTextureDescriptor(rendering::Texture* texture);
		GNS_API rendering::Texture* GetTexture(guid guid);
		GNS_API rendering::Texture* GetTexture(const std::string& name);
		GNS_API rendering::Texture* GetDefaultColorTexture();
		GNS_API rendering::Texture* GetDefaultNormalTexture();

		GNS_API rendering::Mesh* CreateMesh(const std::string& meshFilePath);
		GNS_API rendering::Mesh* GetMesh(guid guid);
		GNS_API rendering::Mesh* GetMesh(const std::string& name);

		GNS_API rendering::Material* CreateMaterial(const std::string& materialFilePath);
		GNS_API rendering::Material* CreateMaterial(rendering::Shader* shader, const std::string& name);
		GNS_API rendering::Material* GetMaterial(guid guid);
		GNS_API rendering::Material* GetMaterial(const std::string& name);

		GNS_API void ResetMaterialTextures(rendering::Material* material);
		GNS_API void UploadMesh(rendering::Mesh* mesh, uint32_t startIndex, uint32_t count);
		GNS_API ImTextureID GetImGuiTexture(TextureHandle handle);
		GNS_API rendering::LightingSettings* GetLightningSettings();
		rendering::Renderer* GetRenderer() const { return m_renderer; };
		void InitSystem() override;
		void UpdateSystem(const float deltaTime) override;
		void FixedUpdate(const float fixedDeltaTime) override;
		void CleanupSystem() override;

	};
}
