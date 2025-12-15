#pragma once
#include "../Object/Object.h"
#include "Vulkan/Utils/VulkanObjects.h"
#include "Handles/Handles.h"

namespace gns::rendering
{
	class Shader : public Object
	{
	public:
		ShaderHandle handle;

		std::string vertexShaderPath;
		std::string fragmentShaderPath;
		gns::rendering::VulkanShader shader;
		bool front = true;

		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& name);
		~Shader() = default;
		void Dispose() override;
	};
}
