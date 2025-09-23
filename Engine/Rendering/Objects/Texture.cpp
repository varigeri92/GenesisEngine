#include "gnspch.h"
#include "Texture.h"
#include "../../AssetDatabase/AssetLoader.h"
#include "../Renderer.h"
#include "../RenderSystem.h"
#include "../../ECS/SystemsManager.h"
#include "../Vulkan/Utils/VulkanObjects.h"


gns::rendering::Texture::Texture(const std::string& name, const std::string& path)
	: Object(name), vulkanImage(nullptr)
{
	//m_guid = gns::hashString(path);
	assetLibrary::LoadTexture(path, *this);
	CreateTexture(data, width, height, 0, false);
	vulkanImage->CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	free(data);
}

gns::rendering::Texture::Texture(const std::string& name)
	: Object(name), vulkanImage(nullptr)
{
	LOG_INFO("Texture created: " + name);
}

gns::rendering::Texture::~Texture()
{
	LOG_INFO("Texture Destroyed: " + name);
	if (vulkanImage != nullptr)
		vulkanImage->Destroy();
}


void gns::rendering::Texture::Dispose()
{
	if(vulkanImage != nullptr)
		vulkanImage->Destroy();

	Object::Dispose();
}

void gns::rendering::Texture::CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t mipLevels, bool keepData)
{
	auto renderSystem = SystemsManager::GetSystem<RenderSystem>();
	vulkanImage = new VulkanImage();
	vulkanImage->CreateImage(data, { width, height, 1 },
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
}
