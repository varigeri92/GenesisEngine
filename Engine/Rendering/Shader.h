#pragma once
#include "../Object/Object.h"
#include "Vulkan/Utils/VulkanObjects.h"

namespace gns::rendering
{
	class Shader : public Object
	{
	public:
		std::string vertexShaderPath;
		std::string fragmentShaderPath;

		gns::rendering::VulkanShader shader;

		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& name);
		~Shader() = default;
		void Dispose() override;
	};
}
