#version 460
#define FRAGMENT_SHADER
#include "defaultshader_vertex.cginc.glsl"
void main() {
    float d = (inFragPosition.z + 1) * 0.5;
    outFragColor = vec4(d);
}