#version 460
#extension GL_EXT_buffer_reference : require
#define VERTEX_SHADER
#include "defaultshader_vertex.cginc.glsl"


void main()
{
    Vertex v = objectBuffer.objects[gl_BaseInstance].vertexBuffer.vertices[gl_VertexIndex];
	mat4 model = objectBuffer.objects[gl_BaseInstance].matrix;

// World position
    vec4 worldPos = model * vec4(v.position, 1.0);

    // World-space normal (if not already normalized you can normalize here)
    vec3 worldNormal = normalize((model * vec4(v.normal, 0.0)).xyz);

    // Offset the caster *towards the light* (or along normal – your choice)
    vec3 offsetPos = worldPos.xyz + worldNormal * sceneData.normalOffset;

    vec4 lightPos = sceneData.dirLightViewProj * vec4(offsetPos, 1.0);

    gl_Position = lightPos;

    // only needed if you still want debug color
    outFragPos = lightPos.xyz;
    outUV = vec2(v.uv_x, v.uv_y);
}