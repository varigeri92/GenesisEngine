#pragma once
#include "glm/glm.hpp"
#include "../../Object/Object.h"
#include "../Handles/Handles.h"
#include "../vulkan/Utils/VulkanObjects.h"
namespace gns::rendering
{
	struct VulkanImage;
}

namespace gns::rendering
{
	enum class TextureType
	{
		Albedo = 0,
		Normal = 1,
		MetallicRoughness = 2,
		AmbientOcclusion = 3,
		Emissive = 4
	};
	struct Texture : public Object
	{
		Texture(const std::string& name);
		Texture(const std::string& name, const std::string& path);

		~Texture() override;
		void Dispose() override;
		void CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t mipLevels = 0, bool keepData = false);

		void* data;
		uint32_t width;
		uint32_t height;
		uint32_t mipLevels = 0;

		TextureHandle handle;

	private:
		bool keepData;
	};
}
