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
		bool front = true;

		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& name);
		~Shader() = default;
		void Dispose() override;
	};

	class ComputeShader : public Object
	{
	public:
		ShaderHandle handle;
		std::string shaderPath;
		bool front = true;

		ComputeShader(const std::string& shaderPath, const std::string& name);
		~ComputeShader() override;
		void Dispose() override;
	};
}
