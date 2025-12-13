#version 460
#extension GL_EXT_buffer_reference : require
#define VERTEX_SHADER
#include "defaultshader_vertex.cginc.glsl"


void main()
{
    Vertex v = objectBuffer.objects[gl_BaseInstance].vertexBuffer.vertices[gl_VertexIndex];
	mat4 model = objectBuffer.objects[gl_BaseInstance].matrix;

    outTangent =(sceneData.dirLightViewProj * model * vec4(v.position, 1.0f));
    vec3 outFragPos = outTangent.xyz + v.normal * 0.001;
    outUV.x = v.uv_x;
	outUV.y = v.uv_y;

	gl_Position = outTangent;
}