#include "gnspch.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED 
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
	m_device->m_gpuSceneDataBuffer = VulkanBuffer::Create(m_device->GetAllocator(), sizeof(GlobalUniformData),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    globalUniform.exposure = 1.f;
    globalUniform.gamma = 1.f;

	//3 default textures, white, grey, black. 1 pixel each
    {
	    uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	    guid guid = hashString("white");
	    Texture* t = Object::CreateWithGuid<Texture>(guid,"white");
	    auto [handle, vkTexture] = m_device->CreateTexture(
	        reinterpret_cast<void*>(&white), { 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
	    t->handle = handle;
    }
    {
	    uint32_t blue = glm::packUnorm4x8(glm::vec4(0.5, 0.5, 1, 1));
	    guid guid = hashString("blue");
	    Texture* t = Object::CreateWithGuid<Texture>(guid, "blue");

	    auto [handle, vkTexture] = m_device->CreateTexture(
	        reinterpret_cast<void*>(&blue), { 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
	    t->handle = handle;
    }
    {
	    uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	    guid guid = hashString("black");
	    Texture* t = Object::CreateWithGuid<Texture>(guid, "black");
	    auto [handle, vkTexture] = m_device->CreateTexture(
	        reinterpret_cast<void*>(&black), { 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
	    t->handle = handle;
    }
    {
	    //checkerboard image
	    constexpr size_t checkerboardSize = 64;
	    uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	    uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	    std::array<uint32_t, checkerboardSize* checkerboardSize > pixels;
	    for (int x = 0; x < checkerboardSize; x++) {
	        for (int y = 0; y < checkerboardSize; y++) {
	            pixels[y * checkerboardSize + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
	        }
	    }
        guid guid = hashString("default_error");
        Texture* t = Object::CreateWithGuid<Texture>(guid, "default_error");
        auto [handle, vkTexture] = m_device->CreateTexture(
            reinterpret_cast<void*>(pixels.data()), { checkerboardSize, checkerboardSize, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
        t->handle = handle;
    }
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

void gns::rendering::Renderer::DestroyTexture(TextureHandle handle)
{
    m_device->DestroyTexture(handle);
}

void gns::rendering::Renderer::DestroyMesh(MeshHandle handle)
{
    m_device->DestroyMesh(handle);
}

void gns::rendering::Renderer::DisposeShader(ShaderHandle handle)
{
    m_device->DisposeShader(handle);
}

void gns::rendering::Renderer::SetbgTexture(TextureHandle handle)
{
    m_device->SetBackgroundTexture(handle);
    LOG_INFO("BG_Texture set!");
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

    _VK_CHECK(vkCreateDescriptorPool(m_device->GetDevice(), &pool_info, nullptr, &imguiPool), "");

    {
		Texture* texture = Object::Find<Texture>("offscreen_texture");
	    VulkanTexture& vkTexture = m_device->GetTexture(texture->handle);
	    DescriptorAllocator allocator;
		DescriptorLayoutBuilder builder;
	    builder.AddBinding(0,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	    vkTexture.setLayout = builder.Build(m_device->GetDevice(), VK_SHADER_STAGE_FRAGMENT_BIT);
	    vkTexture.descriptorSet = allocator.Allocate(m_device->GetDevice(), vkTexture.setLayout, imguiPool);
	    //vkTexture.image.CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	    rendering::DescriptorWriter image_writer;
	    image_writer.WriteImage(0, vkTexture.image.imageView, vkTexture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	    image_writer.UpdateSet(m_device->GetDevice(), vkTexture.descriptorSet);
    }
    {
        Texture* texture = Object::Find<Texture>("shadow_map_debug");
        VulkanTexture& vkTexture = m_device->GetTexture(texture->handle);
        DescriptorAllocator allocator;
        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        vkTexture.setLayout = builder.Build(m_device->GetDevice(), VK_SHADER_STAGE_FRAGMENT_BIT);
        vkTexture.descriptorSet = allocator.Allocate(m_device->GetDevice(), vkTexture.setLayout, imguiPool);
        //vkTexture.image.CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
        rendering::DescriptorWriter image_writer;
        image_writer.WriteImage(0, vkTexture.image.imageView, vkTexture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        image_writer.UpdateSet(m_device->GetDevice(), vkTexture.descriptorSet);
    }

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
    init_info.Device = m_device->GetDevice();
    init_info.Queue = m_device->GetGraphicsQueue();
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;

    //dynamic rendering parameters for imgui to use
    init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = m_device->m_swapchain.GetFormat_ptr();

    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;


    ImGui_ImplVulkan_Init(&init_info);

    //ImGui_ImplVulkan_CreateFontsTexture();

    // add the destroy the imgui created structures
    m_device->m_deletionQueue.Push([=]() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(m_device->GetDevice(), imguiPool, nullptr);
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
uint32_t currenthandle = (uint32_t)-1;
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
            VulkanMesh& vkMesh = m_device->GetMesh(meshComponent.meshes[i]->handle);
            Material* material = meshComponent.materials[i];
	        objects.emplace_back(transform.matrix, vkMesh.vertexBufferAddress, transform.position);
            meshes.push_back(mesh);
            materials.push_back(material);
	        m_device->UpdateUniformBuffer(&material->uniformData, material->buffer);
	        index++;
        }
    }
    m_device->UpdateStorageBuffer(objects.data(), objects.size() * sizeof(ObjectDrawData));

    {
        if (currenthandle == (uint32_t) - 1)
        {
			TextureHandle handle = Object::Find<Texture>("black")->handle;
            SetbgTexture(handle);
            currenthandle = handle.handle;
        }

    	SkyLight sky_light = {};
        auto skyLight_view = SystemsManager::GetRegistry()
            .view<gns::entity::EntityComponent,
            gns::entity::Transform,
            gns::rendering::LightComponent,
            gns::rendering::SkyComponent,
            gns::rendering::ColorComponent>();
        for (auto [entt, entity, transform, light, skyComp, color] : skyLight_view.each())
        {
            if (!entity.active)
                continue;
            globalUniform.pointLight_count++;
            glm::vec3 forward = {
               cosf(transform.rotation.x) * sinf(transform.rotation.y),
               sinf(transform.rotation.x),
               cosf(transform.rotation.x) * cosf(transform.rotation.y)
            };
            sky_light.direction = { forward.x, forward.y, forward.z, transform.rotation.y };
            sky_light.color = { color.color.r, color.color.g, color.color.b, light.intensity};
            Texture* hdr_texture = Object::Get<Texture>(skyComp.hdr);
            if(hdr_texture)
            {
	            if(hdr_texture->handle.handle != currenthandle)
	            {
					SetbgTexture(hdr_texture->handle);
                    currenthandle = hdr_texture->handle.handle;
	            }
            }
        }
        glm::mat4 invProj = glm::inverse(globalUniform.proj);
        glm::mat4 invViewRot = glm::inverse(globalUniform.view);
        m_device->UpdateSkyLightBuffer(&sky_light, sizeof(SkyLight), invProj, invViewRot, sky_light.color, sky_light.direction.w);
    }

    {
	    auto pointLight_view = SystemsManager::GetRegistry()
		.view<gns::entity::EntityComponent,
			gns::entity::Transform,
			gns::rendering::LightComponent,
			gns::rendering::PointLightComponent,
			gns::rendering::ColorComponent>();

		std::vector<PointLight> point_lights = {};
        globalUniform.pointLight_count = 0;
	    point_lights.reserve(pointLight_view.size_hint());
	    for (auto [entt, entity, transform, light, pointLight, color] : pointLight_view.each())
	    {
	        if (!entity.active)
	            continue;
		    globalUniform.pointLight_count++;
	        point_lights.emplace_back(
				transform.position.x, transform.position.y, transform.position.z, pointLight.radius, 
	            color.color.r, color.color.g, color.color.b, light.intensity);
	    }
		m_device->UpdatePointLightBuffer(point_lights.data(), point_lights.size() * sizeof(PointLight));
	}
    {
	    auto dirLightWiev = SystemsManager::GetRegistry()
	        .view<
	        gns::entity::EntityComponent,
	        gns::entity::Transform,
	        gns::rendering::LightComponent,
	        gns::rendering::ColorComponent>(entt::exclude<
                gns::rendering::PointLightComponent, 
                gns::rendering::SpotLightComponent,
				gns::rendering::SkyComponent>);

	    std::vector<DirectionalLight> dir_lights = {};
	    dir_lights.reserve(dirLightWiev.size_hint());
	    globalUniform.dirLight_count = 0;
	    for (auto [entity_handle, entity, transform, light, color] : dirLightWiev.each())
	    {
	        if (!entity.active)
	            continue;

            globalUniform.dirLight_count++;
	        glm::vec3 forward = {
	            cosf(transform.rotation.x) * sinf(transform.rotation.y),
	            sinf(transform.rotation.x),
	            cosf(transform.rotation.x) * cosf(transform.rotation.y)
	        };
	        
	        dir_lights.emplace_back(
	            forward.x, forward.y, forward.z,
	            color.color.r, color.color.g, color.color.b, light.intensity);
            
            BuildDirLightFrustumBasic(forward, globalUniform.camPosition);
	    }
	    m_device->UpdateDirLightBuffer(dir_lights.data(), dir_lights.size() * sizeof(DirectionalLight));
    }
    {
        auto spotLightWiev = SystemsManager::GetRegistry()
            .view<
            gns::entity::EntityComponent,
            gns::entity::Transform,
            gns::rendering::LightComponent,
			gns::rendering::SpotLightComponent,
            gns::rendering::ColorComponent>();

        std::vector<SpotLight> spot_lights = {};
        spot_lights.reserve(spotLightWiev.size_hint());
        globalUniform.spotLight_count = 0;
        for (auto [entity_handle, entity, transform, light, spotLight, color] : spotLightWiev.each())
        {
            if (!entity.active)
                continue;

            globalUniform.spotLight_count++;
            glm::vec3 forward = {
                cosf(transform.rotation.x) * sinf(transform.rotation.y),
                sinf(transform.rotation.x),
                cosf(transform.rotation.x) * cosf(transform.rotation.y)
            };
            forward = glm::normalize(forward);

            spot_lights.emplace_back(
                transform.position.x, transform.position.y, transform.position.z,
                spotLight.distance, spotLight.angle, 
                light.intensity,
                color.color.r, color.color.g, color.color.b,
                forward.x, forward.y, forward.z);
        }
        m_device->UpdateSpotLightBuffer(spot_lights.data(), spot_lights.size() * sizeof(SpotLight));
    }
}

void gns::rendering::Renderer::CreatePipelineForShader(Shader* shader)
{
    if (nullptr != Object::Get<Shader>(shader->m_guid))
    {
        auto [handle, vkShader] = m_device->CreateShader();
        shader->handle = handle;
		m_device->CreatePipeline(handle, *shader);
    }
    else
    {
        LOG_INFO("shader: " + std::to_string(shader->m_guid) + " already has a pipeline created");
    }
}



gns::rendering::Shader* gns::rendering::Renderer::CreateShader(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    size_t shader_guid = hashString(vertexShaderPath+fragmentShaderPath);
    if(std::find(m_shaderCache.begin(), m_shaderCache.end(), shader_guid) != m_shaderCache.end())
        return Object::Get<Shader>(shader_guid);

    Shader* shader = Object::CreateWithGuid<rendering::Shader>(shader_guid, vertexShaderPath, fragmentShaderPath, name);
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
        VulkanTexture& vkTexture = m_device->GetTexture(texture->handle);

        vkTexture.Destroy();
        vkTexture.image = VulkanImage::Create(*m_device, { m_device->m_screen->width, m_device->m_screen->height, 1}
            , VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);


        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        vkTexture.setLayout = builder.Build(m_device->GetDevice(), VK_SHADER_STAGE_FRAGMENT_BIT);

        DescriptorAllocator allocator;
        vkTexture.CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
        vkTexture.descriptorSet = allocator.Allocate(m_device->GetDevice(), vkTexture.setLayout, imguiPool);

        rendering::DescriptorWriter image_writer;
        image_writer.WriteImage(0, vkTexture.image.imageView, vkTexture.sampler,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        image_writer.UpdateSet(m_device->GetDevice(), vkTexture.descriptorSet);
    }
}

void gns::rendering::Renderer::UploadMesh(Mesh* mesh, uint32_t startIndex, uint32_t count) const
{
    mesh->handle = m_device->CreateMesh();
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
    VulkanMesh& vkMesh = m_device->GetMesh(mesh->handle);
    vkMesh.indexBufferRange.startIndex = startIndex;
    vkMesh.indexBufferRange.count = count;
	m_device->UploadMesh(mesh->indices, vertices, vkMesh);

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
    VulkanTexture& vkTexture = m_device->GetTexture(texture->handle);
    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    vkTexture.setLayout = builder.Build(m_device->GetDevice(), VK_SHADER_STAGE_FRAGMENT_BIT);
    vkTexture.descriptorSet = m_device->m_globalDescriptorAllocator.Allocate(m_device->GetDevice(), vkTexture.setLayout);

}

void gns::rendering::Renderer::UpdateTextureDescriptorSet(Texture* texture)
{
    VulkanTexture& vkTexture = m_device->GetTexture(texture->handle);
    DescriptorWriter writer;
    writer.WriteImage(0, vkTexture.image.imageView, vkTexture.sampler,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.UpdateSet(m_device->GetDevice(), vkTexture.descriptorSet);
}

void gns::rendering::Renderer::CreateSampler(Texture* texture)
{
    VulkanTexture& vkTexture = m_device->GetTexture(texture->handle);
    vkTexture.CreateDefaultSampler();
}

void gns::rendering::Renderer::WaitForGPUIddle()
{
	vkDeviceWaitIdle(m_device->GetDevice());
}

void gns::rendering::Renderer::BuildDirLightFrustum(glm::mat4 inverse_viewProj, glm::vec3 fwd)
{
    const glm::vec3 ndcCorners[8] = {
		{ -1.f, -1.f, 0.f }, // near
		{  1.f, -1.f, 0.f },
		{  1.f,  1.f, 0.f },
		{ -1.f,  1.f, 0.f },
		{ -1.f, -1.f, 1.f }, // far
		{  1.f, -1.f, 1.f },
		{  1.f,  1.f, 1.f },
		{ -1.f,  1.f, 1.f },
    };

    glm::vec3 frustumCornersWS[8];
    for (int i = 0; i < 8; ++i)
    {
        glm::vec4 ndc = glm::vec4(ndcCorners[i], 1.0f);
        glm::vec4 world = inverse_viewProj * ndc;
        world /= world.w;
        frustumCornersWS[i] = glm::vec3(world);
    }


    glm::vec3 lightDir = fwd;

    glm::vec3 center(0.0f);
    for (glm::vec3 c : frustumCornersWS)
        center += c;
    center /= 8.0f;

    // Position the light "backwards" along its direction
    float dist = 50.0f; // tweak
    glm::vec3 lightPos = center - lightDir * dist;

    // Pick safe up
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(up, lightDir)) > 0.99f)
        up = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::mat4 lightView = glm::lookAt(lightPos, center, up);

    glm::vec3 frustumLS[8];
    for (int i = 0; i < 8; ++i)
    {
        glm::vec4 ls = lightView * glm::vec4(frustumCornersWS[i], 1.0f);
        frustumLS[i] = glm::vec3(ls);
    }


    glm::vec3 minLS(std::numeric_limits<float>::max());
    glm::vec3 maxLS(-std::numeric_limits<float>::max());

    for (int i = 0; i < 8; ++i)
    {
        minLS = glm::min(minLS, frustumLS[i]);
        maxLS = glm::max(maxLS, frustumLS[i]);
    }


    // optional: add a small padding to avoid clipping at edges
    float padding = 1.0f;
    minLS.x -= padding;
    maxLS.x += padding;
    minLS.y -= padding;
    maxLS.y += padding;

    // For z we usually keep extra range
    float nearPlane = -maxLS.z; // remember: in light space, view looks along -Z
    float farPlane = -minLS.z;

    glm::mat4 lightProj = glm::ortho(
        minLS.x, maxLS.x,
        minLS.y, maxLS.y,
        nearPlane, farPlane
    );

    // Vulkan Y-flip if you do the same for camera projections:
    lightProj[1][1] *= -1.0f;

    globalUniform.dirLightViewProj = lightProj * lightView;
}

void gns::rendering::Renderer::BuildDirLightFrustumBasic(glm::vec3 fwd, glm::vec3 scene_center)
{
    const float halfExtent = m_lightingSettings.halfExtent;
    const float nearPlane = m_lightingSettings.nearPlane;
    const float farPlane = - m_lightingSettings.nearPlane;
    
    glm::vec3 lightDir = glm::normalize(fwd);
    glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };
    glm::vec3 lightPos = scene_center - (fwd);

    glm::vec3 right = glm::normalize(glm::cross(fwd, worldUp));
    glm::vec3 up = glm::normalize(glm::cross(right, fwd));

    glm::mat4 lightView = glm::lookAt(lightPos, scene_center + fwd, worldUp);

    glm::mat4 lightProj = glm::ortho(
        -halfExtent, halfExtent,   // left, right
        -halfExtent, halfExtent,   // bottom, top
        nearPlane, farPlane        // near, far
    );

    lightProj[1][1] *= -1.0f;
    globalUniform.dirLightViewProj = lightProj * lightView;

}

void gns::rendering::Renderer::SetShadowShader(Shader* shader)
{
    m_device->SetShadowShader(shader);
}

gns::rendering::VulkanBuffer gns::rendering::Renderer::CreateUniformBuffer(uint32_t size)
{
    return VulkanBuffer::Create(
        m_device->GetAllocator(),
        size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_MAPPED_BIT
    );
}

gns::rendering::VulkanBuffer gns::rendering::Renderer::CreateStagingBuffer(uint32_t size)
{
    return VulkanBuffer::Create(
        m_device->GetAllocator(),
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        VMA_ALLOCATION_CREATE_MAPPED_BIT
    );
}

gns::rendering::VulkanBuffer gns::rendering::Renderer::CreateIndexBuffer(uint32_t size)
{
    return VulkanBuffer::Create(
        m_device->GetAllocator(),
        size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY
    );
}

gns::rendering::VulkanBuffer gns::rendering::Renderer::CreateVertexBuffer(uint32_t size)
{
    return VulkanBuffer::Create(
        m_device->GetAllocator(),
        size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY
    );
}

TextureHandle gns::rendering::Renderer::CreateTexture(void* data, VkExtent3D size, VkFormat format,
	VkImageUsageFlags usage)
{
    auto[handle, vkTexture] = m_device->CreateTexture(data, size, format, usage);
    return handle;
}

TextureHandle gns::rendering::Renderer::CreateTexture(VkExtent3D size, VkFormat format,
    VkImageUsageFlags usage)
{
    auto [handle, vkTexture] = m_device->CreateTexture(size, format, usage);
    return handle;
}

gns::rendering::VulkanTexture& gns::rendering::Renderer::GetTexture(TextureHandle handle)
{
    return m_device->GetTexture(handle);
}

/*
gns::rendering::VulkanImage gns::rendering::Renderer::CreateImage(void* data, VkExtent3D size, VkFormat format,
                                                                  VkImageUsageFlags usage)
{
    return VulkanImage::Create( data,*m_device,  size, format, usage, false);
}

gns::rendering::VulkanImage gns::rendering::Renderer::CreateImage(VkExtent3D size, VkFormat format,
    VkImageUsageFlags usage)
{
    return VulkanImage::Create(*m_device, size, format, usage, false);
}
*/

