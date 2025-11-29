#include "gnspch.h"
#include "Texture.h"
#include "../../AssetDatabase/AssetLoader.h"
#include "../Renderer.h"
#include "../RenderSystem.h"
#include "../../ECS/SystemsManager.h"
#include "../Vulkan/Utils/VulkanObjects.h"


gns::rendering::Texture::Texture(const std::string& name, const std::string& path)
	: Object(name)
{
	//m_guid = gns::hashString(path);
	assetLibrary::LoadTexture(path, *this);
	CreateTexture(data, width, height, 0, false);
	vulkanImage.CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	free(data);
}

gns::rendering::Texture::Texture(const std::string& name)
	: Object(name), data(nullptr), width(1), height(1), keepData(false)
{
	vulkanImage = VulkanImage::Create();
}

gns::rendering::Texture::~Texture()
{
	LOG_INFO("Texture Destroyed: " + name);
	vulkanImage.Destroy();
}


void gns::rendering::Texture::Dispose()
{
	vulkanImage.Destroy();
	Object::Dispose();
}

void gns::rendering::Texture::CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t mipLevels, bool keepData)
{
	auto renderSystem = SystemsManager::GetSystem<RenderSystem>();
	vulkanImage = renderSystem->GetRenderer()->CreateImage(
		data,
		{ width, height, 1 },
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT);
}
