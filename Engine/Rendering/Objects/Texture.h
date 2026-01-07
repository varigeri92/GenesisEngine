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
		void Apply() override;
		void Apply(void* data);

		void* data {nullptr};
		uint32_t width{1};
		uint32_t height{1};
		uint32_t mipLevels{ 1 };
		TextureHandle handle{Handle::Invalid};
		bool hdr{false};

	private:
		bool keepData{false};
		void DisposeInternal();
		void CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t mipLevels = 0, bool _keepData = false);
	};
}
