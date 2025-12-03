#pragma once
#include "Utils/vkutils.h"
#include "Utils/VulkanObjects.h"
#include "Utils/VkDescriptors.h"
class Screen;

namespace gns::rendering
{
	class Swapchain
	{
	public:
		Swapchain() = default;
		Swapchain(VkDevice m_device,
		VkPhysicalDevice m_physicalDevice,
		VkSurfaceKHR m_surface,
		VmaAllocator m_allocator);
		~Swapchain();

		Swapchain(Swapchain&& other) noexcept;
		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain& operator=(Swapchain&& other) noexcept;


		void Create(Screen* screen);
		void Destroy();
		void Resize(Screen* screen);
		void AllocateDescriptorSet(DescriptorAllocator& descriptor_allocator);


		VkSwapchainKHR& Get() { return m_swapchain; }
		VkSwapchainKHR* Get_ptr() { return &m_swapchain; }
		VkExtent2D GetExtent() { return m_extent; }
		VkFormat GetFormat() { return m_format; }
		VkFormat* GetFormat_ptr() { return &m_format; }
		VulkanImage& GetRenderTargetImage() { return m_renderTarget; }
		VulkanImage& GetDepthImage() { return m_depthImage; }
		VkImage GetImage(size_t index) { return m_images[index]; };
		VkImageView GetImageView(size_t index) { return m_imageViews[index]; };
		VkDescriptorSet* GetDescriptorSet_ptr() { return &m_renderTarget_DS; };
		VkDescriptorSetLayout* GetSetLayout_ptr() { return &m_renderTargetSetLayout; };

	private:
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkSurfaceKHR m_surface;
		VmaAllocator m_allocator;

		VkFormat m_format;
		VkExtent2D m_extent;
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;
		VulkanImage m_renderTarget;
		VkDescriptorSet m_renderTarget_DS;
		VkDescriptorSetLayout m_renderTargetSetLayout;
		VulkanImage m_depthImage;
	};
}
