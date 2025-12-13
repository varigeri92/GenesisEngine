#version 460
#define FRAGMENT_SHADER
#include "defaultshader_vertex.cginc.glsl"
void main() {


    float d = (inTangent.z);
    
    vec3 ndc = inTangent.xyz / inTangent.w;

    // Map to [0,1] UV
    vec2 uv = ndc.xy;// * 0.5 + 0.5;

    // Visualize UV: red=X, green=Y
    outFragColor = vec4(0, d, 0, 1);
    
    //outFragColor = vec4(inFragPosition.xy,0,1);
}