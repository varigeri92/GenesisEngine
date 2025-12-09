#include "gnspch.h"
#include "Mesh.h"

#include "../Renderer.h"
#include "../RenderSystem.h"
#include "../../ECS/SystemsManager.h"

gns::rendering::Mesh::Mesh(std::string name) : Object(name), keepCPU_Data(false),
indices({}), positions({}), normals({}), uvs({}), colors({})
{
}

void gns::rendering::Mesh::DisposeInternal()
{
	auto renderSystem = SystemsManager::GetSystem<RenderSystem>();
	renderSystem->GetRenderer()->DestroyMesh(handle);
}

gns::rendering::Mesh::~Mesh()
{
	DisposeInternal();
}

void gns::rendering::Mesh::Dispose()
{
	DisposeInternal();
	Object::Dispose();
}
