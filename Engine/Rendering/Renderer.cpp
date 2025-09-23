﻿#include "gnspch.h"
#include "Renderer.h"

#include <fstream>

#include "Vulkan/Device.h"
#include "../ECS/Entity.h"
#include "../ECS/Component.h"
#include "../ECS/SystemsManager.h"
#include "../AssetDatabase/AssetLoader.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "../Window/Window.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "Objects/Material.h"
#include "Objects/Mesh.h"
#include "Objects/Lights.h"
#include "../Window/WindowSystem.h"
#include "spirv_cross/spirv_cross.hpp"

gns::rendering::Device* m_device;

constexpr size_t DEFAULT_VECTROR_MEMORY_RESERVE = 100;


gns::rendering::Renderer::Renderer(Screen* screen) : m_screen(screen)
{
    m_shaderCache = {};
    m_shaderCache.reserve(DEFAULT_VECTROR_MEMORY_RESERVE);
	m_device = new Device(screen);
	m_device->m_gpuSceneDataBuffer = VulkanBuffer::CreateBuffer(sizeof(GlobalUniformData),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    globalUniform.ambientColor = { 1,1,1,0.1f };
    globalUniform.exposure = 1.f;
    globalUniform.gamma = 1.f;

    guid guid = hashString("white");
    Texture* t = Object::CreateWithGuid<Texture>(guid,"white");
    t->vulkanImage = &m_device->_whiteImage;
    t->vulkanImage->CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    t->width = 1;
    t->height = 1;
    
    guid = hashString("black");
    t = Object::CreateWithGuid<Texture>(guid, "black");
    t->vulkanImage = &m_device->_blackImage;
    t->vulkanImage->CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    t->width = 1;
    t->height = 1;

    guid = hashString("blue");
    t = Object::CreateWithGuid<Texture>(guid, "blue");
    t->vulkanImage = &m_device->_blueImage;
    t->vulkanImage->CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    t->width = 1;
    t->height = 1;
}

gns::rendering::Renderer::~Renderer()
{
	delete(m_device);
}

gns::rendering::Texture* gns::rendering::Renderer::GetDefaultTexture(const std::string& textureName)
{
    guid guid = hashString(textureName);
    return Object::Get<Texture>(guid);
}

VkDescriptorPool imguiPool;
void gns::rendering::Renderer::InitImGui()
{

    VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;

    _VK_CHECK(vkCreateDescriptorPool(m_device->sDevice, &pool_info, nullptr, &imguiPool), "");

	Texture* texture = Object::Find<Texture>("offscreen_texture");

    DescriptorAllocator allocator;
	DescriptorLayoutBuilder builder;
    builder.AddBinding(0,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    texture->vulkanImage->setlayout = builder.Build(Device::sDevice, VK_SHADER_STAGE_FRAGMENT_BIT);
    texture->vulkanImage->CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    texture->vulkanImage->texture_descriptorSet = allocator.Allocate(Device::sDevice, texture->vulkanImage->setlayout, imguiPool);

    rendering::DescriptorWriter image_writer;
    image_writer.WriteImage(0, texture->vulkanImage->imageView, texture->vulkanImage->sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    image_writer.UpdateSet(rendering::Device::sDevice, texture->vulkanImage->texture_descriptorSet);

    // 2: initialize imgui library
    // this initializes the core structures of imgui
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); //(void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows


    Window* window = SystemsManager::GetSystem<WindowSystem>()->GetWindow();
    // this initializes imgui for SDL
    ImGui_ImplSDL2_InitForVulkan(window->sdlWindow);


    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_device->m_instance;
    init_info.PhysicalDevice = m_device->m_physicalDevice;
    init_info.Device = Device::sDevice;
    init_info.Queue = Device::sGraphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;

    //dynamic rendering parameters for imgui to use
    init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_device->m_swapchainFormat;

    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;


    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();

    // add the destroy the imgui created structures
    m_device->m_deletionQueue.Push([=]() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(m_device->sDevice, imguiPool, nullptr);
        });
}

void gns::rendering::Renderer::BeginGuiFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void gns::rendering::Renderer::UpdateBuffers()
{

}

void gns::rendering::Renderer::BuildDrawData()
{
    constexpr uint32_t UniformBufferBinding = 0;
    constexpr uint32_t ObjectBufferBinding = 1;
    constexpr uint32_t PointLightBufferBinding = 2;
    constexpr uint32_t SpotLightBufferBinding = 3;


    objects.clear();
    objectIndices.clear();
    meshes.clear();
    materials.clear();

    auto object_view = SystemsManager::GetRegistry()
        .view<gns::entity::EntityComponent, gns::entity::Transform, gns::entity::MeshComponent>();

    size_t view_size = object_view.size_hint();
    objects.reserve(view_size);
    objectIndices.reserve(view_size);

    size_t index = 0;
    for (auto [entt, entity, transform, meshComponent] : object_view.each())
    {
        if (!entity.active)
            continue;
        for (size_t i = 0; i < meshComponent.meshes.size(); i++)
        {
	        objectIndices.push_back(index);
            Mesh* mesh = meshComponent.meshes[i];
            Material* material = meshComponent.materials[i];
	        objects.emplace_back(transform.matrix, mesh->DrawData.vertexBufferAddress, transform.position);
            meshes.push_back(mesh);
            materials.push_back(material);
	        m_device->UpdateUniformBuffer(&material->uniformData, material->buffer);
	        index++;
        }
    }

    auto pointLight_view = SystemsManager::GetRegistry()
	.view<gns::entity::EntityComponent,
		gns::entity::Transform,
		gns::rendering::PointLightComponent,
		gns::rendering::ColorComponent>();

	std::vector<PointLight> point_lights = {};
    point_lights.reserve(pointLight_view.size_hint());
    for (auto [entt, entity, transform, pointLight, color] : pointLight_view.each())
    {
        if (!entity.active)
            continue;
        point_lights.emplace_back(
			transform.position.x, transform.position.y, transform.position.z, pointLight.radius, 
            color.color.r, color.color.g, color.color.b, pointLight.intensity);
    }
    globalUniform.pointLight_count = point_lights.size();
    globalUniform.spotLight_count = 0;
    globalUniform.dirLight_count = 0;

    m_device->UpdateStorageBuffer(objects.data(), objects.size() * sizeof(ObjectDrawData));
    m_device->UpdatePointLightBuffer(point_lights.data(), point_lights.size() * sizeof(PointLight));
    //m_device->UpdateSpotLightBuffer(nullptr, 0);
}

void gns::rendering::Renderer::CreatePipelineForShader(Shader* shader)
{
    if(nullptr != Object::Get<Shader>(shader->m_guid))
    {
		m_device->CreatePipeline(*shader);
    }
    else
    {
        LOG_INFO("shader: " + std::to_string(shader->m_guid) + " already has a pipeline created");
    }
}



gns::rendering::Shader* gns::rendering::Renderer::CreateShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    LOG_INFO(vertexShaderPath + fragmentShaderPath);
    size_t shader_guid = hashString(vertexShaderPath+fragmentShaderPath);
    if(std::find(m_shaderCache.begin(), m_shaderCache.end(), shader_guid) != m_shaderCache.end())
        return Object::Get<Shader>(shader_guid);
    
    
    Shader* shader = Object::CreateWithGuid<rendering::Shader>(shader_guid, vertexShaderPath, fragmentShaderPath, "default_shader");
    CreatePipelineForShader(shader);
	m_shaderCache.push_back(shader_guid);
	return shader;
    
}

gns::rendering::Shader* gns::rendering::Renderer::ReCreateShader(const guid guid)
{
    Shader* shader = Object::Get<Shader>(guid);
    CreatePipelineForShader(shader);
    return shader;
}

void gns::rendering::Renderer::Draw()
{
    BuildDrawData();
	m_device->UpdateGlobalUbo(&globalUniform, sizeof(GlobalUniformData));
	m_device->Draw(objects, objectIndices, meshes, materials);
    if(m_device->m_screen->resized)
    {
        m_device->m_screen->resized = false;
        Texture* texture = Object::Find<Texture>("offscreen_texture");

        
        texture->vulkanImage->Destroy();
        texture->vulkanImage = new VulkanImage();
        texture->vulkanImage->CreateImage({ m_device->m_screen->width, m_device->m_screen->height, 1}
            , VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);


        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        texture->vulkanImage->setlayout = builder.Build(Device::sDevice, VK_SHADER_STAGE_FRAGMENT_BIT);

        DescriptorAllocator allocator;
        texture->vulkanImage->CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
        texture->vulkanImage->texture_descriptorSet = allocator.Allocate(Device::sDevice, texture->vulkanImage->setlayout, imguiPool);

        rendering::DescriptorWriter image_writer;
        image_writer.WriteImage(0, texture->vulkanImage->imageView, texture->vulkanImage->sampler, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        image_writer.UpdateSet(rendering::Device::sDevice, texture->vulkanImage->texture_descriptorSet);
    }
}

void gns::rendering::Renderer::UploadMesh(Mesh* mesh) const
{
	std::vector<Vertex> vertices = {};
	vertices.resize(mesh->positions.size());
	for (size_t i = 0; i<mesh->positions.size(); i++)
	{
		vertices[i].position = { mesh->positions[i] };
		vertices[i].uv_x = mesh->uvs[i].x;
		vertices[i].normal = { mesh->normals[i] };
		vertices[i].uv_y = mesh->uvs[i].y;
        vertices[i].color = { mesh->colors[i] };
        vertices[i].tangent = { mesh->tangents[i],0 };
        vertices[i].biTangent = { mesh->biTangents[i],0 };
	}
	m_device->UploadMesh(mesh->indices, vertices, mesh->DrawData);
	if(!mesh->keepCPU_Data)
	{
		mesh->indices.clear();
		mesh->positions.clear();
		mesh->normals.clear();
		mesh->colors.clear();
		mesh->uvs.clear();
	}
}

void gns::rendering::Renderer::CreateTextureDescriptorSet(Texture* texture)
{
    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    texture->vulkanImage->setlayout = builder.Build(Device::sDevice, VK_SHADER_STAGE_FRAGMENT_BIT);
    texture->vulkanImage->texture_descriptorSet = m_device->m_globalDescriptorAllocator.Allocate(Device::sDevice, texture->vulkanImage->setlayout);

}

void gns::rendering::Renderer::UpdateTextureDescriptorSet(Texture* texture)
{
    DescriptorWriter writer;
    writer.WriteImage(0, texture->vulkanImage->imageView, texture->vulkanImage->sampler,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.UpdateSet(Device::sDevice, texture->vulkanImage->texture_descriptorSet);
}

void gns::rendering::Renderer::WaitForGPUIddle()
{
	vkDeviceWaitIdle(Device::sDevice);
}

