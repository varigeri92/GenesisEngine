#include "gnspch.h"
#include "Mesh.h"

gns::rendering::Mesh::Mesh(std::string name) : Object(name), keepCPU_Data(false),
indices({}), positions({}), normals({}), uvs({}), colors({})
{
}

gns::rendering::Mesh::~Mesh()
{
	//DrawData.vertexBuffer.Destroy();
	//DrawData.indexBuffer.Destroy();
}

void gns::rendering::Mesh::Dispose()
{
	Object::Dispose();
}
