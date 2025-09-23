#include "gnspch.h"
#include "Material.h"


gns::rendering::Material::Material(const std::string& name) :Object(name), shader(nullptr), uniformData() {}

gns::rendering::Material::~Material() = default;

void gns::rendering::Material::Dispose()
{
	Object::Dispose();
}

