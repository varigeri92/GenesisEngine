
#version 460
#extension GL_EXT_buffer_reference : require
#define VERTEX_SHADER 
#include "DefaultShader_Vertex.cginc.glsl"

void main() 
{
	Vertex v = objectBuffer.objects[gl_BaseInstance].vertexBuffer.vertices[gl_VertexIndex];

	mat4 model = objectBuffer.objects[gl_BaseInstance].matrix;
	//output data
	
	outColor = v.color.xyz;
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
	outFragPos = vec3(model * vec4(v.position, 1.0));

	mat3 model_TI = mat3(transpose(inverse(model)));

	//mat3 model_TI = mat3(model);
	
	outNormal = model_TI * v.normal;
	outTangent = vec4(model_TI * v.tangent.xyz, v.tangent.w);
	outBiTangent = vec4(model_TI * v.biTangent.xyz, v.biTangent.w);
	
	gl_Position = sceneData.viewproj * model * vec4(v.position, 1.0f);
}
