﻿#include "gnspch.h"
#include "Shader.h"


gns::rendering::Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& name) :
	Object(name),
	vertexShaderPath(vertexShaderPath),
	fragmentShaderPath(fragmentShaderPath)
{
	LOG_INFO("Shader ctor");
}

void gns::rendering::Shader::Dispose()
{
	shader.Destroy();
	Object::Dispose();
}
