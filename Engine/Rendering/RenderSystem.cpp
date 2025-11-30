#include "gnspch.h"
#include "RenderSystem.h"
#include "GuiWindowDrawer.h"
#include "Renderer.h"
#include "../ECS/Component.h"
#include "../ECS/SystemsManager.h"
#include "imgui.h"
#include "../AssetDatabase/AssetLoader.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "Objects/Lights.h"
#include "Shader.h"
#include "../Utils/FileSystemUtils.h"
#include "../Window/Screen.h"

void ReadShaderAttributes(gns::rendering::Shader* shader)
{
    
}

void gns::RenderSystem::SetActiveCamera(rendering::Camera* camera, entity::Transform* transform)
{
    m_camera = camera;
    m_cameraTransform = transform;
}
#pragma region resources
gns::rendering::Shader* gns::RenderSystem::CreateShader(const std::string& vertexShaderPath,
	const std::string& fragmentShaderPath)
{
    return  m_renderer->CreateShader(vertexShaderPath, fragmentShaderPath);
}

gns::rendering::Shader* gns::RenderSystem::ReCreateShader(const guid guid)
{
    return  m_renderer->ReCreateShader(guid);
}

gns::rendering::Shader* gns::RenderSystem::GetShader(guid guid)
{
    return Object::Get<rendering::Shader>(guid);
}

gns::rendering::Shader* gns::RenderSystem::GetShader(const std::string& name)
{
	return Object::Find<rendering::Shader>(name);
}

gns::rendering::Texture* gns::RenderSystem::CreateTexture(const std::string& texturePath)
{
    if(!fileUtils::FileExists(texturePath))
        return nullptr;

    guid  guid = hashString(texturePath);
    rendering::Texture* texture = Object::CreateWithGuid<rendering::Texture>(guid, texturePath, texturePath);
    return texture;
}

void gns::RenderSystem::CreateTextureDescriptor(rendering::Texture* texture)
{
    m_renderer->CreateTextureDescriptorSet(texture);
}

void gns::RenderSystem::UpdateTextureDescriptor(rendering::Texture* texture)
{
    m_renderer->UpdateTextureDescriptorSet(texture);
}

gns::rendering::Texture* gns::RenderSystem::GetTexture(guid guid)
{
	return Object::Get<rendering::Texture>(guid);
}

gns::rendering::Texture* gns::RenderSystem::GetTexture(const std::string& name)
{
	return Object::Find<rendering::Texture>(name);
}

gns::rendering::Texture* gns::RenderSystem::GetDefaultColorTexture()
{
    return m_renderer->GetDefaultTexture("white");
}

gns::rendering::Texture* gns::RenderSystem::GetDefaultNormalTexture()
{
    return m_renderer->GetDefaultTexture("blue");
}

gns::rendering::Mesh* gns::RenderSystem::CreateMesh(const std::string& meshFilePath)
{
	return Object::Create<rendering::Mesh>("");
}

gns::rendering::Mesh* gns::RenderSystem::GetMesh(guid guid)
{
	return Object::Get<rendering::Mesh>(guid);
}

gns::rendering::Mesh* gns::RenderSystem::GetMesh(const std::string& name)
{
	return Object::Find<rendering::Mesh>(name);
}

gns::rendering::Material* gns::RenderSystem::CreateMaterial(const std::string& materialFilePath)
{
	rendering::Material* material = Object::Create<rendering::Material>("default_material");
    material->buffer = m_renderer->CreateUniformBuffer(sizeof(rendering::MaterialUniformData));
    material->uniformData = {};
    return material;
}

gns::rendering::Material* gns::RenderSystem::CreateMaterial(rendering::Shader* shader, const std::string& name)
{
    rendering::Material* material = Object::Create<rendering::Material>(name);
    material->buffer = m_renderer->CreateUniformBuffer(sizeof(rendering::MaterialUniformData));
    material->uniformData = {};
    material->shader = shader;

    return material;
}

gns::rendering::Material* gns::RenderSystem::GetMaterial(guid guid)
{
	return Object::Get<rendering::Material>(guid);
}

gns::rendering::Material* gns::RenderSystem::GetMaterial(const std::string& name)
{
	return Object::Find<rendering::Material>(name);
}

void gns::RenderSystem::ResetMaterialTextures(rendering::Material* material)
{
    rendering::Texture* albedoTexture = m_renderer->GetDefaultTexture("white");
    rendering::Texture* normalTexture = m_renderer->GetDefaultTexture("blue");
    rendering::Texture* metallicRoughnessTexture = m_renderer->GetDefaultTexture("white");
    rendering::Texture* aoTexture = m_renderer->GetDefaultTexture("white");
    rendering::Texture* emissionTexture = m_renderer->GetDefaultTexture("white");

    material->textures.resize(5);
    material->textures[0] = albedoTexture;
    material->textures[1] = normalTexture;
    material->textures[2] = metallicRoughnessTexture;
    material->textures[3] = aoTexture;
    material->textures[4] = emissionTexture;
}

void gns::RenderSystem::UploadMesh(rendering::Mesh* mesh)
{
    m_renderer->UploadMesh(mesh);
}

ImTextureID gns::RenderSystem::GetImGuiTexture(TextureHandle handle)
{
    return  reinterpret_cast<ImTextureID>(m_renderer->GetTexture(handle).descriptorSet);
}
#pragma endregion

void gns::RenderSystem::InitSystem()
{
    m_renderer = new gns::rendering::Renderer(m_renderScreen);
	m_renderer->InitImGui();
    m_offScreenRenderTargetTexture = Object::Find<rendering::Texture>("offscreen_texture");

    const std::string v_shader_path = R"(Shaders\colored_triangle_mesh.vert)";
    const std::string f_shader_path = R"(Shaders\tex_image.frag)";
    rendering::Shader* shader = CreateShader(v_shader_path, f_shader_path);
    rendering::Material* default_material = CreateMaterial(shader, "default_material");
    ResetMaterialTextures(default_material);

    if (m_offScreenRenderTargetTexture == nullptr)
    {
        LOG_ERROR("Could not find texture!");
    }
}
bool keep_aspect = true;
float aspect = (float)16 / (float)9;
float r_aspect = (float)9 / (float)16;
void gns::RenderSystem::UpdateSystem(float deltaTime)
{
    BeginGuiFrame();
    gns::GuiWindowDrawer::DrawWindows();

    ImGui::EndFrame();
    UpdateCamera();
	m_renderer->Draw();
    EndGuiFrame();
}

void gns::RenderSystem::FixedUpdate(float fixedDeltaTime)
{
	
}

void gns::RenderSystem::CleanupSystem()
{
	m_renderer->WaitForGPUIddle();
    Object::ClearAll();
}

gns::RenderSystem::RenderSystem(Screen* screen)
{
	if(screen != nullptr)
        m_renderScreen = screen;
	else
        m_renderScreen = new Screen(640, 480, 1.f, ((float)640 / (float)480), false);
	
}

gns::rendering::Texture* gns::RenderSystem::GetRenderTargetTexture()
{
    return m_offScreenRenderTargetTexture;
}

ImTextureID gns::RenderSystem::GetRenderTargetTextureID()
{
	return GetImGuiTexture(m_offScreenRenderTargetTexture->handle);
}

Screen* gns::RenderSystem::GetTargetScren()
{
    return m_renderScreen;
}

void gns::RenderSystem::SetTargetScreenSize(uint32_t width, uint32_t height)
{
    m_renderScreen->width = width;
    m_renderScreen->height = height;
}

void gns::RenderSystem::UpdateCamera()
{
    m_renderer->globalUniform.view = m_camera->m_view;
    m_renderer->globalUniform.proj = m_camera->m_projection;
    m_renderer->globalUniform.viewProj = m_camera->m_cameraMatrix;
    m_renderer->globalUniform.camPosition = { m_cameraTransform->position.x, m_cameraTransform->position.y,m_cameraTransform->position.z, 1 };
}


void gns::RenderSystem::BeginGuiFrame()
{
    m_renderer->BeginGuiFrame();

    m_renderer->globalUniform.sunlightDirection = { 1,0,0,1 };
    m_renderer->globalUniform.sunlightColor = { 0,1,0,1 };
}

void gns::RenderSystem::EndGuiFrame()
{
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}