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
	assets::LoadTexture(path, *this, &hdr);
}

gns::rendering::Texture::Texture(const std::string& name)
	: Object(name)
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

void gns::rendering::Texture::Apply()
{
	if (!data)
	{
		LOG_ERROR("Cannot apply Texture because no data was set!");
		LOG_VERBOSE("Call 'Texture::Apply(void* data);' to set the Raw texture data instead of 'Texture::Apply();'");
		return;
	}
	auto* renderer = SystemsManager::GetSystem<RenderSystem>()->GetRenderer();
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	if (hdr)
		format = VK_FORMAT_R32G32B32A32_SFLOAT;
	handle = renderer->CreateTexture(data, { width, height, 1 }, format, VK_IMAGE_USAGE_SAMPLED_BIT);
	renderer->CreateTextureDescriptorSet(this);
	renderer->UpdateTextureDescriptorSet(this);
	renderer->CreateSampler(this);
	free(data);
}

void gns::rendering::Texture::Apply(void* _data)
{
	data = _data;
	Apply();
}

void gns::rendering::Texture::DisposeInternal()
{
	auto renderSystem = SystemsManager::GetSystem<RenderSystem>();
	renderSystem->GetRenderer()->DestroyTexture(handle);
}
