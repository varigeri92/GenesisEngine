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
	assetLibrary::LoadTexture(path, *this);
	CreateTexture(data, width, height, 0, false);
	free(data);
}

gns::rendering::Texture::Texture(const std::string& name)
	: Object(name), data(nullptr), width(1), height(1), keepData(false)
{}

gns::rendering::Texture::~Texture()
{
	DisposeInternal();
}


void gns::rendering::Texture::Dispose()
{
	DisposeInternal();
	Object::Dispose();
}

void gns::rendering::Texture::CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t mipLevels, bool keepData)
{
	auto renderSystem = SystemsManager::GetSystem<RenderSystem>();
	handle = renderSystem->GetRenderer()->
	CreateTexture(data, { width, height, 1 }, VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_USAGE_SAMPLED_BIT);
}

void gns::rendering::Texture::DisposeInternal()
{
	auto renderSystem = SystemsManager::GetSystem<RenderSystem>();
	renderSystem->GetRenderer()->DestroyTexture(handle);
}
