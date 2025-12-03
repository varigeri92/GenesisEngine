#include "gnspch.h"
#include "Swapchain.h"
#include "VkBootstrap.h"
#include "../../Window/Screen.h"
#include "Utils/VkDescriptors.h"

gns::rendering::Swapchain::Swapchain( VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VmaAllocator allocator):

	m_device(device),
	m_physicalDevice(physicalDevice),
	m_surface(surface),
	m_allocator(allocator) {}

gns::rendering::Swapchain::~Swapchain()
{
}

gns::rendering::Swapchain::Swapchain(Swapchain&& other) noexcept
{
	m_device = other.m_device;
	m_physicalDevice = other.m_physicalDevice;
	m_surface = other.m_surface;
	m_allocator = other.m_allocator;
}

gns::rendering::Swapchain& gns::rendering::Swapchain::operator=(Swapchain&& other) noexcept
{
    m_device = other.m_device;
    m_physicalDevice = other.m_physicalDevice;
    m_surface = other.m_surface;
    m_allocator = other.m_allocator;
    return *this;
}

void gns::rendering::Swapchain::Create(Screen* screen)
{
    vkb::SwapchainBuilder swapchainBuilder{ m_physicalDevice, m_device, m_surface };

    m_format = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        //.use_default_format_selection()
        .set_desired_format(VkSurfaceFormatKHR{ .format = m_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        //use vsync present mode
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(screen->width, screen->height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    m_extent = vkbSwapchain.extent;
    m_swapchain = vkbSwapchain.swapchain;
    m_images = vkbSwapchain.get_images().value();
    m_imageViews = vkbSwapchain.get_image_views().value();

    VkExtent3D drawImageExtent = {
        m_extent.width,
        m_extent.height,
        1
    };
    screen->width = m_extent.width;
    screen->height = m_extent.height;
    screen->aspectRatio = (float)m_extent.width / (float)m_extent.height;
    screen->updateRenderTargetTarget = true;
    //hardcoding the draw format to 32 bit float
    m_renderTarget.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_renderTarget.imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo rimg_info = utils::ImageCreateInfo(m_renderTarget.imageFormat, drawImageUsages, drawImageExtent);

    //for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    vmaCreateImage(m_allocator, &rimg_info, &rimg_allocinfo, &m_renderTarget.image, &m_renderTarget.allocation, nullptr);

    //build a image-view for the draw image to use for rendering
    VkImageViewCreateInfo rview_info = utils::ImageViewCreateInfo(m_renderTarget.imageFormat, m_renderTarget.image, VK_IMAGE_ASPECT_COLOR_BIT);

    _VK_CHECK(vkCreateImageView(m_device, &rview_info, nullptr, &m_renderTarget.imageView), "");


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

void gns::rendering::Swapchain::Destroy()
{
    vkDestroyImageView(m_device, m_renderTarget.imageView, nullptr);
    vmaDestroyImage(m_allocator, m_renderTarget.image, m_renderTarget.allocation);

    vkDestroyImageView(m_device, m_depthImage.imageView, nullptr);
    vmaDestroyImage(m_allocator, m_depthImage.image, m_depthImage.allocation);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    for (int i = 0; i < m_imageViews.size(); i++) {
        vkDestroyImageView(m_device, m_imageViews[i], nullptr);
    }
}

void gns::rendering::Swapchain::Resize(Screen* screen)
{
    vkDeviceWaitIdle(m_device);

    Destroy();
    Create(screen);

    DescriptorWriter writer;
    writer.WriteImage(0, m_renderTarget.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.UpdateSet(m_device, m_renderTarget_DS);
}
