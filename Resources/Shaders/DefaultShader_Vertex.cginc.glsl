#ifdef VERTEX_SHADER

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outFragPos;
layout (location = 4) out vec4 outTangent;
layout (location = 5) out vec4 outBiTangent;
layout (location = 6) out mat3 outTBN;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
    vec4 tangent;
    vec4 biTangent;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{
    Vertex vertices[];
};

struct ObjectData{
    mat4 matrix;
    VertexBuffer vertexBuffer;
    vec3 position;
};

//push constants block
layout( push_constant ) uniform constants
{
    mat4 render_matrix;
    uint index;
} PushConstants;

//all object matrices
layout(std140, set = 1, binding = 1) readonly buffer ObjectBuffer{
    ObjectData objects[];
} objectBuffer;

#endif

#ifdef FRAGMENT_SHADER
//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inFragPosition;
layout (location = 4) in vec4 inTangent;
layout (location = 5) in vec4 inBiTangent;
layout (location = 6) in mat3 inTBN;

//output write
layout (location = 0) out vec4 outFragColor;


layout(set = 0, binding = 0) uniform MaterialData{
    vec4 albedo;
    vec4 metallic_roughness_AO_emission;
}material;

layout(set = 0, binding = 1) uniform sampler2D albedoTexture;
layout(set = 0, binding = 2) uniform sampler2D normalTexture;
layout(set = 0, binding = 3) uniform sampler2D metallicRoughnessTexture;
layout(set = 0, binding = 4) uniform sampler2D aoTexture;
layout(set = 0, binding = 5) uniform sampler2D emissiveTexture;

struct PointLight
{
    vec4 position; // .w = radius
    vec4 color; // .w = intensity
};

struct SpotLight
{
    vec4 position; // .w = radius
    vec4 color; // .w = intensity
    vec4 direction; // .w = angle
};

layout(std140, set = 1, binding = 2) readonly buffer PointLightBuffer{
    PointLight objects[];
} pointLights;

layout(std140, set = 1, binding = 3) readonly buffer SpotLightBuffer{
    SpotLight objects[];
} spotLights;
#endif

//Shared
layout(set = 1, binding = 0) uniform  SceneData{
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 camPosition;
    vec4 ambientColor;
    vec4 sunlightDirection; // w for sun power
    vec4 sunlightColor;
    uint pointLight_count;
    uint spotLight_count;
    uint dirLight_count;
    float exposure;
    float gamma;
} sceneData;
