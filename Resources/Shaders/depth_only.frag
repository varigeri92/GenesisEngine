#version 460
#define FRAGMENT_SHADER
#include "defaultshader_vertex.cginc.glsl"
void main() {
    float d = (inFragPosition.z);
    outFragColor = vec4(d, d, d, 1);
}