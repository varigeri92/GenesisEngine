//> all
#version 460

//shader input
layout (location = 0) in vec3 inColor;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 0) uniform GPUSceneData{
	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor; // .w = intensity
	vec4 sunlightDirection; // w for sun power
	vec4 sunlightColor;
}sceneData;

void main() 
{
	//return red
	outFragColor = sceneData.ambientColor;//vec4(inColor,1.0f);
}
//< all