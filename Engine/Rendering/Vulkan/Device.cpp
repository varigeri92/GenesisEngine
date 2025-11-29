#include "gnspch.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "Device.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <array>
#include "VkBootstrap.h"
#include "Utils/VulkanObjects.h"
#include "imgui.h"
#include "PipelineBuilder.h"
#include "../Renderer.h"
#include "../../imgui/backends/imgui_impl_vulkan.h"
#include "../AssetDatabase/AssetLoader.h"
#include "Utils/VkDescriptors.h"
#include "../ECS/Entity.h"
#include "../ECS/Component.h"
#include "../ECS/SystemsManager.h"
#include <algorithm>

#include "../RenderSystem.h"
#include "../../Utils/PathHelper.h"
#include "../../Window/Screen.h"
#include "../../Window/WindowSystem.h"


#pragma region DeletionQueue
void gns::rendering::DeletionQueue::Push(std::function<void()>&& function)
{
	deletors.push_back(function);
}

void gns::rendering::DeletionQueue::Flush()
{
	for (auto it = deletors.rbegin(); it != deletors.rend(); it++) 
	{
		(*it)();
	}

	deletors.clear();
}
#pragma endregion

#pragma region DescriptorLayoutBuilders:
void gns::rendering::DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
{
    VkDescriptorSetLayoutBinding newbind{};
    newbind.binding = binding;
    newbind.descriptorCount = 1;
    newbind.descriptorType = type;

    bindings.push_back(newbind);
}

void gns::rendering::DescriptorLayoutBuilder::Clear()
{
    bindings.clear();
}

VkDescriptorSetLayout gns::rendering::DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages,
	void* pNext, VkDescriptorSetLayoutCreateFlags flags)
{
    for (auto& b : bindings) {
        b.stageFlags |= shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    info.pNext = pNext;

    info.pBindings = bindings.data();
    info.bindingCount = (uint32_t)bindings.size();
    info.flags = flags;

    VkDescriptorSetLayout set;
    _VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set),"Failed To create Descriptor set layout.");

    return set;
}

void gns::rendering::DescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets,
	std::span<PoolSizeRatio> poolRatios)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.type,
            .descriptorCount = uint32_t(ratio.ratio * maxSets)
            });
    }

    VkDescriptorPoolCreateInfo pool_info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    pool_info.flags = 0;
    pool_info.maxSets = maxSets;
    pool_info.poolSizeCount = (uint32_t)poolSizes.size();
    pool_info.pPoolSizes = poolSizes.data();

    vkCreateDescriptorPool(device, &pool_info, nullptr, &pool);
}

void gns::rendering::DescriptorAllocator::ClearDescriptors(VkDevice device)
{
    vkResetDescriptorPool(device, pool, 0);
}

void gns::rendering::DescriptorAllocator::DestroyPool(VkDevice device)
{
    vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet gns::rendering::DescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds;
    _VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds),"Failed To allocate descriptor set");

    return ds;
}

VkDescriptorSet gns::rendering::DescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout,
	VkDescriptorPool _pool)
{
    VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = _pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    
    VkDescriptorSet ds;
    _VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds), "Failed To allocate descriptor set");

    return ds;
}
#pragma endregion
gns::rendering::ImmeduateSubmitStruct gns::rendering::Device::sImmediateSubmitStruct = {};
// Device Code:
gns::rendering::Device::Device(Screen* screen) : m_screen(screen), m_imageCount(2), m_imageIndex(0)
{
    currentBackgroundEffect = 0;
    
    InitVulkan();
    CreateSwapchain();
    offscreen_Texture = Object::Create<Texture>("offscreen_texture");
    offscreen_Texture->vulkanImage = VulkanImage::Create(
        *this,
        m_renderTargetImage.imageExtent, 
        VK_FORMAT_R8G8B8A8_UNORM, 
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    InitCommands();
    InitSyncStructures();
    InitDescriptors();

	//-> Init on demand:
    InitPipelines();

    constexpr size_t DEFAULT_STORAGE_BUFFER_OBJECT_COUNT = 500;

    m_objectStorageBuffer = VulkanBuffer::Create(m_allocator, sizeof(ObjectDrawData) * DEFAULT_STORAGE_BUFFER_OBJECT_COUNT,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    m_pointLightStorageBuffer = VulkanBuffer::Create(m_allocator, sizeof(PointLight) * DEFAULT_STORAGE_BUFFER_OBJECT_COUNT,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    m_spotLightStorageBuffer = VulkanBuffer::Create(m_allocator, sizeof(SpotLight) * DEFAULT_STORAGE_BUFFER_OBJECT_COUNT,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}


gns::rendering::Device::~Device()
{
    vkDeviceWaitIdle(m_device);
    vkDestroyDescriptorSetLayout(m_device, m_perFrameDescriptorLayout, nullptr);

    for (size_t i = 0; i < m_imageCount; i++)
    {
        vkDestroyCommandPool(m_device, m_frames[i].commandPool, nullptr);
        vkDestroyFence(m_device, m_frames[i].renderFence, nullptr);
        vkDestroySemaphore(m_device, m_frames[i].renderSemaphore, nullptr);
        vkDestroySemaphore(m_device, m_frames[i].presentSemaphore, nullptr);
        m_frames[i].deletionQueue.Flush();
    }

    DestroySwapchain();
    m_deletionQueue.Flush();

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkb::destroy_swapchain(m_vkb_swapchain);
    vkb::destroy_device(m_vkb_device);
}

#pragma region Device_Initalization_Functions

void gns::rendering::Device::InitPipelines()
{
    InitBackgroundPipelines();
}

void gns::rendering::Device::InitBackgroundPipelines()
{
    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(ComputePushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &m_renderTargetSetLayout;
    computeLayout.setLayoutCount = 1;
    computeLayout.pPushConstantRanges = &pushConstant;
    computeLayout.pushConstantRangeCount = 1;

    _VK_CHECK(vkCreatePipelineLayout(m_device, &computeLayout, nullptr, &m_gradientPipelineLayout),"Failed To create Pipeline Layout");

    const std::string gradient_ShaderPath = PathHelper::FromResourcesRelative(R"(Shaders\gradient_color.comp.spv)");
    VkShaderModule gradientShader;
    if (!utils::LoadShaderModule(gradient_ShaderPath.c_str(), m_device, &gradientShader)) {
        LOG_ERROR("Failed To load Shader: " + gradient_ShaderPath);
    }

    const std::string sky_ShaderPath = PathHelper::FromResourcesRelative("R(Shaders\sky.comp.spv)");
    VkShaderModule skyShader;
    if (!utils::LoadShaderModule(gradient_ShaderPath.c_str(), m_device, &skyShader)) {
        LOG_ERROR("Failed To load Shader: " + sky_ShaderPath);
    }


    VkPipelineShaderStageCreateInfo stageinfo{};
    stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageinfo.pNext = nullptr;
    stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageinfo.module = gradientShader;
    stageinfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_gradientPipelineLayout;
    computePipelineCreateInfo.stage = stageinfo;


    ComputeEffect gradient;
    gradient.layout = m_gradientPipelineLayout;
    gradient.name = "gradient";
    gradient.data = {};
    gradient.data.data1 = glm::vec4(0.588f, 0.964f, 1, 1);
    gradient.data.data2 = glm::vec4(0.436f, 0.436f, 0.436f, 1);

    _VK_CHECK(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline), 
        "Failed to create Compute Pipeline.");

    computePipelineCreateInfo.stage.module = skyShader;

    ComputeEffect sky;
    sky.layout = m_gradientPipelineLayout;
    sky.name = "sky";
    sky.data = {};
    //default sky parameters
    sky.data.data1 = glm::vec4(0.1, 0.2, 0.4, 1);
    sky.data.data1 = glm::vec4(0, 0, 0, 1);

    _VK_CHECK(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline),
        "Failed to create Compute Pipeline");

    backgroundEffects.push_back(gradient);
    backgroundEffects.push_back(sky);


    vkDestroyShaderModule(m_device, gradientShader, nullptr);
    vkDestroyShaderModule(m_device, skyShader, nullptr);

    m_deletionQueue.Push([&]() {
        vkDestroyPipelineLayout(m_device, m_gradientPipelineLayout, nullptr);
        vkDestroyPipeline(m_device, backgroundEffects[0].pipeline, nullptr);
        vkDestroyPipeline(m_device, backgroundEffects[1].pipeline, nullptr);
        });
}

void gns::rendering::Device::CreatePipeline(Shader& shader)
{
    LOG_INFO("CreatingPipeline for Shader:" + std::to_string(shader.m_guid));
    shader.shader.device = m_device;
    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(PushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    {
        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        builder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        builder.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        builder.AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        builder.AddBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        shader.shader.m_descriptorSetLayout = builder.Build(m_device, VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    VkDescriptorSetLayout layouts[] = { shader.shader.m_descriptorSetLayout, m_perFrameDescriptorLayout };

    VkPipelineLayoutCreateInfo pipeline_layout_info = utils::PipelineLayoutCreateInfo();
    pipeline_layout_info.pPushConstantRanges = &bufferRange;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pSetLayouts = layouts;
    pipeline_layout_info.setLayoutCount = 2;

    _VK_CHECK(vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &shader.shader.m_pipelineLayout),
        "Failed to crete pipeline Layout.");
    {
        PipelineBuilder pipelineBuilder(this);
        pipelineBuilder.m_pipelineLayout = shader.shader.m_pipelineLayout;
        pipelineBuilder.SetShaders(shader);
        pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
        pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        pipelineBuilder.SetMultisampling(false);
        pipelineBuilder.SetBlending(PipelineBuilder::BlendingMode::disabled);
        pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_LESS_OR_EQUAL);
        pipelineBuilder.SetColorAttachmentFormat(m_renderTargetImage.imageFormat);
        pipelineBuilder.SetDepthFormat(m_depthImage.imageFormat);
        shader.shader.m_pipeline = pipelineBuilder.BuildPipeline(m_device);
    }
}

bool gns::rendering::Device::InitVulkan()
{
        vkb::InstanceBuilder builder;

#ifdef VK_LOG
    auto inst_ret = builder.request_validation_layers(true).set_debug_callback(VulkanDebugCallback)
        .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        .set_app_name("Awesome Vulkan Application")
        .set_engine_name("Excellent Game Engine")
        .require_api_version(1, 3, 0)
        .set_app_version(0, 0, 1)
        .build();
#else
    auto inst_ret = builder.request_validation_layers(false).set_app_name("Awesome Vulkan Application")
        .set_engine_name("Excellent Game Engine")
        .require_api_version(1, 2, 0)
        .set_app_version(0, 0, 1)
        .build();
#endif

    if (!inst_ret) {
        LOG_ERROR("Failed to create Vulkan instance. Error: " + inst_ret.error().message());
        return false;
    }
    m_vkb_instance = inst_ret.value();
    m_instance = m_vkb_instance.instance;

    Window* window = SystemsManager::GetSystem<WindowSystem>()->GetWindow();

    SDL_Vulkan_CreateSurface(window->sdlWindow, m_instance, &m_surface);


    //vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    //VK_KHR_shader_draw_parameters

    //vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    VkPhysicalDeviceVulkan11Features features11{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
    features11.shaderDrawParameters = VK_TRUE;


    vkb::PhysicalDeviceSelector selector{ m_vkb_instance };
    auto phys_ret = selector.set_surface(m_surface)
        .set_minimum_version(1, 3) // require a vulkan 1.1 capable device
        .require_dedicated_transfer_queue()
        .set_required_features_13(features13)
        .set_required_features_12(features12)
		.set_required_features_11(features11)
        .select();
    if (!phys_ret) {
        LOG_ERROR("Failed to select Vulkan Physical Device. Error: " + phys_ret.error().message());
        return false;
    }
    m_physicalDevice = phys_ret.value();
    LOG_INFO("Phym_device Selected!");
    vkb::DeviceBuilder device_builder{ phys_ret.value() };

    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        LOG_ERROR("Failed to create Vulkan device. Error: " + dev_ret.error().message());
        return false;
    }
    m_vkb_device = dev_ret.value();

    // Get the VkDevice handle used in the rest of a vulkan application
    m_device = dev_ret.value().device;
    LOG_INFO("LogicalDevice Created!");
    // Get the graphics queue with a helper function
    auto graphics_queue_ret = m_vkb_device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_ret) {
        LOG_ERROR("Failed to get graphics queue. Error: " + graphics_queue_ret.error().message());
        return false;
    }
    m_graphicsQueue = graphics_queue_ret.value();
    m_graphicsFamilyIndex = m_vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    auto transfer_queue_ret = m_vkb_device.get_queue(vkb::QueueType::transfer);
    if (!transfer_queue_ret) {
        LOG_ERROR("Failed to get graphics queue. Error: " + transfer_queue_ret.error().message());
        return false;
    }
    m_transferQueue = transfer_queue_ret.value();
    m_transferFamilyIndex = m_vkb_device.get_queue_index(vkb::QueueType::transfer).value();

    auto present_queue_ret = m_vkb_device.get_queue(vkb::QueueType::present);
    if (!present_queue_ret) {
        LOG_ERROR("Failed to get graphics queue. Error: " + present_queue_ret.error().message());
        return false;
    }
    m_presentQueue = present_queue_ret.value();
    m_presentFamilyIndex = m_vkb_device.get_queue_index(vkb::QueueType::present).value();


    auto compute_queue_ret = m_vkb_device.get_queue(vkb::QueueType::compute);
    if (!compute_queue_ret) {
        LOG_ERROR("Failed to get graphics queue. Error: " + compute_queue_ret.error().message());
        return false;
    }
    m_computeQueue = compute_queue_ret.value();
    m_computeFamilyIndex = m_vkb_device.get_queue_index(vkb::QueueType::compute).value();


    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = m_physicalDevice;
    allocatorCreateInfo.device = m_device;
    allocatorCreateInfo.instance = m_instance;
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
    m_deletionQueue.Push([&]() {vmaDestroyAllocator(m_allocator); });

    return true;
}

void gns::rendering::Device::CreateSwapchain()
{
    vkb::SwapchainBuilder swapchainBuilder{ m_physicalDevice, m_device, m_surface };

    m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        //.use_default_format_selection()
        .set_desired_format(VkSurfaceFormatKHR{ .format = m_swapchainFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        //use vsync present mode
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(m_screen->width, m_screen->height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    m_swapchainExtent = vkbSwapchain.extent;
    m_swapchain = vkbSwapchain.swapchain;
    m_images = vkbSwapchain.get_images().value();
    m_imageViews = vkbSwapchain.get_image_views().value();

    VkExtent3D drawImageExtent = {
        m_swapchainExtent.width,
        m_swapchainExtent.height,
        1
    };
    m_screen->width = m_swapchainExtent.width;
    m_screen->height = m_swapchainExtent.height;
    m_screen->aspectRatio = (float)m_swapchainExtent.width / (float)m_swapchainExtent.height;
    m_screen->updateRenderTargetTarget = true;
    //hardcoding the draw format to 32 bit float
    m_renderTargetImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_renderTargetImage.imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo rimg_info = utils::ImageCreateInfo(m_renderTargetImage.imageFormat, drawImageUsages, drawImageExtent);

    //for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    vmaCreateImage(m_allocator, &rimg_info, &rimg_allocinfo, &m_renderTargetImage.image, &m_renderTargetImage.allocation, nullptr);

    //build a image-view for the draw image to use for rendering
    VkImageViewCreateInfo rview_info = utils::ImageViewCreateInfo(m_renderTargetImage.imageFormat, m_renderTargetImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

    _VK_CHECK(vkCreateImageView(m_device, &rview_info, nullptr, &m_renderTargetImage.imageView),"");


    m_depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
    m_depthImage.imageExtent = drawImageExtent;
    VkImageUsageFlags depthImageUsages{};
    depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageCreateInfo dimg_info = utils::ImageCreateInfo(m_depthImage.imageFormat, depthImageUsages, drawImageExtent);

    //allocate and create the image
    vmaCreateImage(m_allocator, &dimg_info, &rimg_allocinfo, &m_depthImage.image, &m_depthImage.allocation, nullptr);

    //build a image-view for the draw image to use for rendering
    VkImageViewCreateInfo dview_info = utils::ImageViewCreateInfo(m_depthImage.imageFormat, m_depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    _VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &m_depthImage.imageView), "Failed to create Depth Image");
}

void gns::rendering::Device::ResizeSwapchain()
{
	vkDeviceWaitIdle(m_device);

	DestroySwapchain();
	CreateSwapchain();

    DescriptorWriter writer;
    writer.WriteImage(0, m_renderTargetImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.UpdateSet(m_device, m_renderTargetDescriptor);
}

void gns::rendering::Device::DestroySwapchain()
{

    vkDestroyImageView(m_device, m_renderTargetImage.imageView, nullptr);
    vmaDestroyImage(m_allocator, m_renderTargetImage.image, m_renderTargetImage.allocation);

    vkDestroyImageView(m_device, m_depthImage.imageView, nullptr);
    vmaDestroyImage(m_allocator, m_depthImage.image, m_depthImage.allocation);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    for (int i = 0; i < m_imageViews.size(); i++) {
        vkDestroyImageView(m_device, m_imageViews[i], nullptr);
    }
}
void gns::rendering::Device::InitDescriptors()
{
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
    {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    };
    m_globalDescriptorAllocator.InitPool(m_device, 10, sizes);
    {
        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        m_renderTargetSetLayout = builder.Build(m_device, VK_SHADER_STAGE_COMPUTE_BIT);
    }
    m_renderTargetDescriptor = m_globalDescriptorAllocator.Allocate(m_device, m_renderTargetSetLayout);
    DescriptorWriter writer;
    writer.WriteImage(0, m_renderTargetImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.UpdateSet(m_device, m_renderTargetDescriptor);


    for (uint32_t i = 0; i < m_imageCount; i++) {
        std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
        };
        m_frames[i].frameDescriptors = DescriptorAllocatorGrowable{};
        m_frames[i].frameDescriptors.Init(m_device, 1000, frame_sizes);
        m_deletionQueue.Push([&, i]() {
            m_frames[i].frameDescriptors.DestroyPools(m_device);
            });
    }
    //Per frame Descriptor Layout:
    {
        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // Scene Data Buffer
        builder.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER); // object buffer
        builder.AddBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER); // point light buffer
        builder.AddBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER); // spot light buffer
        m_perFrameDescriptorLayout = builder.Build(m_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    m_deletionQueue.Push([&]() {
        m_globalDescriptorAllocator.DestroyPool(m_device);
        vkDestroyDescriptorSetLayout(m_device, m_renderTargetSetLayout, nullptr);
        });
}

void gns::rendering::Device::InitCommands()
{
    VkCommandPoolCreateInfo commandPoolInfo = gns::rendering::utils::CommandPoolCreateInfo(
        m_graphicsFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    m_frames.resize(m_imageCount);
    for (uint32_t i = 0; i < m_imageCount; i++) {

        _VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_frames[i].commandPool), "Failed to create Commandpool");
        VkCommandBufferAllocateInfo cmdAllocInfo = utils::CommandBufferAllocateInfo(m_frames[i].commandPool);
        _VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_frames[i].mainCommandBuffer), "Failed to allocate command buffer");
    }

    //immediate Commands:
    _VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &sImmediateSubmitStruct._immCommandPool), "Failed toi create immediate CommandPool.");

    // allocate the command buffer for immediate submits
    VkCommandBufferAllocateInfo cmdAllocInfo = utils::CommandBufferAllocateInfo(sImmediateSubmitStruct._immCommandPool, 1);

    _VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &sImmediateSubmitStruct._immCommandBuffer), "Failed to Allocate immediate CommandBuffer");

    m_deletionQueue.Push([=]() {
        vkDestroyCommandPool(m_device, sImmediateSubmitStruct._immCommandPool, nullptr);
        });
}

void gns::rendering::Device::InitSyncStructures()
{
    VkFenceCreateInfo fenceCreateInfo = utils::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = utils::SemaphoreCreateInfo();

    for (uint32_t i = 0; i < m_imageCount; i++) {
        _VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_frames[i].renderFence), "Failed to create fence");

        _VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i].presentSemaphore), "Failed to create PresentSemaphoire");
        _VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i].renderSemaphore), "Failed to create renderSemaphore");
    }

    _VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &sImmediateSubmitStruct._immFence), "Failed to Create Immediate Fence.");
    m_deletionQueue.Push([=]() { vkDestroyFence(m_device, sImmediateSubmitStruct._immFence, nullptr); });
}
#pragma endregion

#pragma region BufferUpdates

void gns::rendering::Device::ImmediateSubmit(VkDevice device, VkQueue queue, std::function<void(VkCommandBuffer cmd)>&& function)
{
    _VK_CHECK(vkResetFences(device, 1, &sImmediateSubmitStruct._immFence), "");
    _VK_CHECK(vkResetCommandBuffer(sImmediateSubmitStruct._immCommandBuffer, 0), "");
    VkCommandBuffer cmd = sImmediateSubmitStruct._immCommandBuffer;
    VkCommandBufferBeginInfo cmdBeginInfo = utils::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    _VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "");
    function(cmd);
    _VK_CHECK(vkEndCommandBuffer(cmd), "");
    VkCommandBufferSubmitInfo cmdinfo = utils::CommandBufferSubmitInfo(cmd);
    VkSubmitInfo2 submit = utils::SubmitInfo(&cmdinfo, nullptr, nullptr);
    _VK_CHECK(vkQueueSubmit2(queue, 1, &submit, sImmediateSubmitStruct._immFence), "");
    _VK_CHECK(vkWaitForFences(device, 1, &sImmediateSubmitStruct._immFence, true, 9999999999), "Fence Time Out: immediateFence");
}

void gns::rendering::Device::UpdateUniformBuffer(void* data, const VulkanBuffer& buffer)
{
    void* dataPtr = (buffer.allocation->GetMappedData());
    if (dataPtr != nullptr)
        memcpy(dataPtr, data, buffer.bufferSize);
    else
        std::cout << "SHIT!";
}

void gns::rendering::Device::UpdateGlobalUbo(void* data, size_t size)
{
    void* dataPtr = (m_gpuSceneDataBuffer.allocation->GetMappedData());
    memcpy(dataPtr, data, size);

}

void gns::rendering::Device::UpdateStorageBuffer(void* data, size_t size)
{
    void* dataPtr = (m_objectStorageBuffer.allocation->GetMappedData());
    memcpy(dataPtr, data, size);

}

void gns::rendering::Device::UpdatePointLightBuffer(void* data, size_t size)
{
    void* dataPtr = (m_pointLightStorageBuffer.allocation->GetMappedData());
    memcpy(dataPtr, data, size);
}

void gns::rendering::Device::UpdateSpotLightBuffer(void* data, size_t size)
{
    void* dataPtr = (m_spotLightStorageBuffer.allocation->GetMappedData());
    memcpy(dataPtr, data, size);
}
#pragma endregion

#pragma region Drawing

bool gns::rendering::Device::BeginDraw(uint32_t& swapchainImageIndex)
{
    m_screen->resized = false;

    _VK_CHECK(vkWaitForFences(m_device, 1, &GetCurrentFrame().renderFence, true, 1000000000), "Wait for fence time out... ");
    _VK_CHECK(vkResetFences(m_device, 1, &GetCurrentFrame().renderFence), "");
    GetCurrentFrame().deletionQueue.Flush();
    GetCurrentFrame().frameDescriptors.ClearPools(m_device);
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain,
        1000000000, GetCurrentFrame().presentSemaphore, nullptr, &swapchainImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_screen->resized = true;
        return false;
    }
    return true;
}

void gns::rendering::Device::StartFrame(VkCommandBuffer& cmd)
{
    cmd = GetCurrentFrame().mainCommandBuffer;
    _VK_CHECK(vkResetCommandBuffer(cmd, 0), "");

    VkCommandBufferBeginInfo cmdBeginInfo = utils::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);


    drawExtent.height = std::min(m_swapchainExtent.height, m_renderTargetImage.imageExtent.height) * 1.f;
    drawExtent.width = std::min(m_swapchainExtent.width, m_renderTargetImage.imageExtent.width) * 1.f;

    _VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "");

    utils::TransitionImage(cmd, m_renderTargetImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void gns::rendering::Device::DrawBackground(VkCommandBuffer cmd)
{
    ComputeEffect& effect = backgroundEffects[currentBackgroundEffect];
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);
    vkCmdBindDescriptorSets(cmd, 
        VK_PIPELINE_BIND_POINT_COMPUTE, m_gradientPipelineLayout, 0, 1, 
        &m_renderTargetDescriptor, 0, nullptr);
    vkCmdPushConstants(cmd, m_gradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
    vkCmdDispatch(cmd, 
        std::ceil(static_cast<float>(drawExtent.width) / 16.0f), 
        std::ceil(static_cast<float>(drawExtent.height) / 16.0f), 1);
}

void gns::rendering::Device::SetDrawStructs(VkCommandBuffer cmd)
{

    VkRenderingAttachmentInfo colorAttachment = utils::AttachmentInfo(m_renderTargetImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttachment = utils::DepthAttachmentInfo(m_depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = utils::RenderingInfo(m_swapchainExtent, &colorAttachment, &depthAttachment);

    vkCmdBeginRendering(cmd, &renderInfo);

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(m_swapchainExtent.width);
    viewport.height = static_cast<float>(m_swapchainExtent.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = m_swapchainExtent.width;
    scissor.extent.height = m_swapchainExtent.height;
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void gns::rendering::Device::UpdatePerFrameDescriptors(VkDescriptorSet& perFrameSet)
{
    DescriptorWriter writer;
    writer.WriteBuffer(0, m_gpuSceneDataBuffer.buffer,
        m_gpuSceneDataBuffer.bufferSize, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    writer.WriteBuffer(1, m_objectStorageBuffer.buffer,
        m_objectStorageBuffer.bufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	writer.WriteBuffer(2, m_pointLightStorageBuffer.buffer,
        m_pointLightStorageBuffer.bufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	writer.WriteBuffer(3, m_spotLightStorageBuffer.buffer,
        m_spotLightStorageBuffer.bufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	writer.UpdateSet(m_device, perFrameSet);
}

void gns::rendering::Device::UpdateMaterialData(VkDescriptorSet& set, Material& material)
{
    DescriptorWriter image_writer;
    image_writer.WriteBuffer(0, material.buffer.buffer, material.buffer.bufferSize, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    for(size_t i = 0; i<material.textures.size(); i++)
    {
	    const VulkanImage* vulkanImage = &(material.textures[i]->vulkanImage);
		image_writer.WriteImage(i+1, vulkanImage->imageView, vulkanImage->sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }
    image_writer.UpdateSet(m_device, set);
}


void gns::rendering::Device::Draw(
    std::vector<ObjectDrawData>& objects, 
    std::vector<size_t>& indices, 
    std::vector<Mesh*>& meshes,
    std::vector<Material*>& materials)
{
    uint32_t swapchainImageIndex;
    if (!BeginDraw(swapchainImageIndex))
        return;

    VkCommandBuffer cmd;
    StartFrame(cmd);

    VkClearColorValue clear = { 0,0,0 };
    VkImageSubresourceRange subResourceRange = {};
    subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subResourceRange.baseMipLevel = 0;
    subResourceRange.levelCount = 1;
    subResourceRange.baseArrayLayer = 0;
    subResourceRange.layerCount = 1;

	//vkCmdClearColorImage(cmd, m_images[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clear, 1, &subResourceRange);

    DrawBackground(cmd);

    utils::TransitionImage(cmd, m_renderTargetImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	utils::TransitionImage(cmd, m_depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    SetDrawStructs(cmd);

    DrawGeometry(cmd, objects, indices, meshes, materials);

    utils::TransitionImage(cmd, m_renderTargetImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	utils::TransitionImage(cmd, m_images[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    utils::CopyImageToImage(cmd, m_renderTargetImage.image, m_images[swapchainImageIndex], drawExtent, m_swapchainExtent);

    utils::TransitionImage(cmd, offscreen_Texture->vulkanImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    utils::CopyImageToImage(cmd, m_renderTargetImage.image, offscreen_Texture->vulkanImage.image, drawExtent, drawExtent);
    utils::TransitionImage(cmd, offscreen_Texture->vulkanImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	DrawImGui(cmd, m_imageViews[swapchainImageIndex]);
    utils::TransitionImage(cmd, m_images[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	_VK_CHECK(vkEndCommandBuffer(cmd),"");

    EndFrame(cmd, swapchainImageIndex);
    EndDraw();
}

void gns::rendering::Device::DrawGeometry(VkCommandBuffer cmd, 
											std::vector<ObjectDrawData>& objects,
											std::vector<size_t>& indices, 
											std::vector<Mesh*>& meshes,
											std::vector<Material*>& materials)
{
    m_currentBoundShader = nullptr;
    m_currentMaterial = nullptr;
    
    VkDescriptorSet perFrameDescriptor = GetCurrentFrame().frameDescriptors.Allocate(m_device, m_perFrameDescriptorLayout);
    UpdatePerFrameDescriptors(perFrameDescriptor);

    for(size_t i = 0; i < indices.size(); i++)
    {
        size_t objectIndex = indices[i];
        Material* material = materials[i];
        Mesh* mesh = meshes[i];

        if (material->shader != m_currentBoundShader)
        {
		    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->shader->shader.m_pipeline);
		    m_currentBoundShader = material->shader;
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                material->shader->shader.m_pipelineLayout, 1, 1, &perFrameDescriptor, 0, nullptr);
        }

        if (material != m_currentMaterial)
        {
            VkDescriptorSet imageSet = GetCurrentFrame().frameDescriptors.Allocate(m_device, material->shader->shader.m_descriptorSetLayout);
            UpdateMaterialData(imageSet, *material);

            std::vector<VkDescriptorSet> sets = { imageSet };

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                material->shader->shader.m_pipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);
            m_currentMaterial = material;
        }

        PushConstants push_constants;
        push_constants.vertexBuffer = objects[objectIndex].vertexBufferAddress;
        push_constants.worldMatrix = objects[objectIndex].objectMatrix;

        vkCmdPushConstants(cmd, material->shader->shader.m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &push_constants);
        vkCmdBindIndexBuffer(cmd, mesh->DrawData.indexBuffer.buffer,
            0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, mesh->indexBufferRange.count,
            1, mesh->indexBufferRange.startIndex, 0, objectIndex);
    }
    vkCmdEndRendering(cmd);
}


void gns::rendering::Device::DrawImGui(VkCommandBuffer cmd, VkImageView targetImageView)
{
	VkRenderingAttachmentInfo colorAttachment = utils::AttachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = utils::RenderingInfo(m_swapchainExtent, &colorAttachment, nullptr);
	vkCmdBeginRendering(cmd, &renderInfo);
    ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	vkCmdEndRendering(cmd);
}


void gns::rendering::Device::EndFrame(VkCommandBuffer& cmd, uint32_t& swapchainImageIndex)
{

    VkCommandBufferSubmitInfo cmdinfo = utils::CommandBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo signalInfo = utils::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().renderSemaphore);
    VkSemaphoreSubmitInfo waitInfo = utils::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().presentSemaphore);

    VkSubmitInfo2 submit = utils::SubmitInfo(&cmdinfo, &signalInfo, &waitInfo);

    _VK_CHECK(vkQueueSubmit2(m_graphicsQueue, 1, &submit, GetCurrentFrame().renderFence), "Submit Failed");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    VkResult presentResult = vkQueuePresentKHR(m_graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
        m_screen->resized = true;
    }

    m_imageIndex++;
    m_imageIndex = m_imageIndex % m_imageCount;
}

void gns::rendering::Device::EndDraw()
{
    if (m_screen->resized) {
        ResizeSwapchain();
    }
}

gns::rendering::FrameData& gns::rendering::Device::GetCurrentFrame()
{
    return m_frames[m_imageIndex];
}

#pragma  endregion

void gns::rendering::Device::UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices, VulkanMesh& vulkanMesh)
{
    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

    vulkanMesh = {};
    vulkanMesh.indexBuffer = VulkanBuffer::Create(m_allocator, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    vulkanMesh.vertexBuffer = VulkanBuffer::Create(m_allocator, vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAdressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = vulkanMesh.vertexBuffer.buffer };

    vulkanMesh.vertexBufferAddress = vkGetBufferDeviceAddress(m_device, &deviceAdressInfo);

    VulkanBuffer staging = VulkanBuffer::Create(m_allocator, vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = staging.allocation->GetMappedData();
    memcpy(data, vertices.data(), vertexBufferSize);
    memcpy(static_cast<char*>(data) + vertexBufferSize, indices.data(), indexBufferSize);

    ImmediateSubmit(m_device, m_graphicsQueue, [&](VkCommandBuffer cmd) {
        VkBufferCopy vertexCopy{ 0 };
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer, vulkanMesh.vertexBuffer.buffer, 1, &vertexCopy);

        VkBufferCopy indexCopy{ 0 };
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer, vulkanMesh.indexBuffer.buffer, 1, &indexCopy);
        });
}



uint32_t gns::rendering::Device::GetMemoryType(
    uint32_t typeBits, 
    VkMemoryPropertyFlags properties,
    VkBool32* memTypeFound) const
{
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memTypeFound)
                {
                    *memTypeFound = true;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if (memTypeFound)
    {
        *memTypeFound = false;
        return 0;
    }
    else
    {
        throw std::runtime_error("Could not find a matching memory type");
    }
}

