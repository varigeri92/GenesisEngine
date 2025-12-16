#include "gnspch.h"
#include "Shader.h"

#include "RenderSystem.h"


gns::rendering::Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& name) :
	Object(name),
	vertexShaderPath(vertexShaderPath),
	fragmentShaderPath(fragmentShaderPath)
{}

void gns::rendering::Shader::Dispose()
{
	RenderSystem* renderSystem = SystemsManager::GetSystem<RenderSystem>();
	renderSystem->DisposeShader(handle);
	Object::Dispose();
}

gns::rendering::ComputeShader::ComputeShader(const std::string& shaderPath, const std::string& name)
	:Object(name), shaderPath(shaderPath)
{}

gns::rendering::ComputeShader::~ComputeShader() = default;

void gns::rendering::ComputeShader::Dispose()
{
	RenderSystem* renderSystem = SystemsManager::GetSystem<RenderSystem>();
	renderSystem->DisposeShader(handle);
	Object::Dispose();
}
