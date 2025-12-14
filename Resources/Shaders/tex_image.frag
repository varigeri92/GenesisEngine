#version 460
#define FRAGMENT_SHADER
#include "defaultshader_vertex.cginc.glsl"

#define PI 3.1415926535897932384626433832795
#define ALBEDO pow(texture(albedoTexture, inUV).rgb, vec3(2.2)) * material.albedo.xyz

// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculateNormal(vec3 tangentNormal)
{
    vec3 N = normalize(inNormal);
    vec3 T = normalize(inTangent.xyz);
    vec3 B = normalize(inBiTangent.xyz);
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

// =========================
// Lighting helper functions
// =========================

// Returns the BRDF contribution for a single light direction (without radiance).
// You still need to multiply by the light's radiance outside this function.
vec3 EvaluateBRDF(vec3 N, vec3 V, vec3 L,
                  vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    float denom = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    vec3 spec   = (NDF * G * F) / denom;

    // BRDF * NdotL (without light radiance)
    return (kD * albedo / PI + spec) * NdotL;
}
float edgeFade(vec2 uv)
{
    float d = min(min(uv.x, uv.y), min(1.0 - uv.x, 1.0 - uv.y));
    float fadeStart = 0.1;
    return clamp(d / fadeStart, 0.0, 1.0);
}

float ComputeShadowFactor(vec3 worldPos, vec3 N, vec3 L)
{
    vec4 lightClip = sceneData.dirLightViewProj * vec4(worldPos, 1.0);
    vec3 ndc = lightClip.xyz / lightClip.w;

    // stay safe: treat outside clip as lit
    if (ndc.z > 1.0 || ndc.z < 0.0)
        return 1.0;

    vec2 uv = ndc.xy * 0.5 + 0.5;
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        return 1.0;

    float currentDepth = ndc.z;

    // -------- slope-scaled bias (light-angle only) --------
    float cosTheta = max(dot(N, L), 0.0);

    // derive bias from worldPerTexel, but use saner multipliers
    float worldPerTexel = (2.0 * sceneData.halfExtent) / float(sceneData.shadowMapSize);

    float minBias    = worldPerTexel * sceneData.shadowBias;   // e.g. shadowBias = 0.05
    float slopeBias  = worldPerTexel * sceneData.slopeScale * (1.0 - cosTheta); // e.g. slopeScale = 2.0

    float bias = minBias + slopeBias;
    // ------------------------------------------------------

    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float visibility = 0.0;
    int kernel = 1;  // 1 = 3x3, 2 = 5x5, etc.

    for (int x = -kernel; x <= kernel; ++x)
    {
        for (int y = -kernel; y <= kernel; ++y)
        {
            vec2 sampleUV = uv + vec2(x, y) * texelSize;
            sampleUV = clamp(sampleUV, vec2(0.0), vec2(1.0));

            float shadowDepth = texture(shadowMap, sampleUV).r;
            visibility += (currentDepth - bias > shadowDepth) ? 0.0 : 1.0;
        }
    }

    visibility /= float((kernel * 2 + 1) * (kernel * 2 + 1));
    float fade = edgeFade(uv);
    visibility = mix(1.0, visibility, fade);
    return visibility;
}


vec3 AccumulateDirectionalLights(vec3 N, vec3 V,
                                 vec3 albedo, float metallic, float roughness, vec3 F0,
                                 vec3 worldPos)
{
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < sceneData.dirLight_count; ++i)
    {
        vec3 lightForward = dirLights.objects[i].direction.xyz;

        // Fragment-to-light direction
        vec3 L = -lightForward;

        vec3 lightColor = dirLights.objects[i].color.xyz;
        float intensity = dirLights.objects[i].color.w;

        // No attenuation for directional lights
        vec3 radiance = lightColor * intensity;

        vec3 brdf = EvaluateBRDF(N, V, L, albedo, metallic, roughness, F0);

        float shadow = ComputeShadowFactor(worldPos, N, L);
        Lo += brdf * radiance * shadow;
    }

    return Lo;
}

vec3 AccumulatePointLights(vec3 N, vec3 V,
                           vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < sceneData.pointLight_count; ++i)
    {
        vec3 lightPos = pointLights.objects[i].position.xyz;
        vec3 L        = lightPos - inFragPosition;
        float distance = length(L);
        L = L / distance; // normalize

        float range = pointLights.objects[i].position.w;

        // Hard cutoff (cheap + simple)
        if (distance > range)
            continue;

        // Base inverse-square falloff
        float invDist2 = 1.0 / (distance * distance);

        // Optional smooth range falloff so it fades out nicely near the edge
        float x = clamp(distance / range, 0.0, 1.0);
        float rangeFalloff = 1.0 - x * x;      // quadratic
        rangeFalloff *= rangeFalloff;          // ^4 for smoother edge

        float attenuation = invDist2 * rangeFalloff;

        vec3 lightColor = pointLights.objects[i].color.xyz;
        float intensity = pointLights.objects[i].color.w;
        vec3 radiance   = lightColor * intensity * attenuation;

        vec3 brdf = EvaluateBRDF(N, V, L, albedo, metallic, roughness, F0);

        Lo += brdf * radiance;
    }

    return Lo;
}

vec3 AccumulateSpotLights(vec3 N, vec3 V,
                          vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < sceneData.spotLight_count; ++i)
    {
        SpotLight light = spotLights.objects[i];

        // Fragment-to-light direction (same convention as point lights)
        vec3 lightPos = light.position.xyz;
        vec3 L        = lightPos - inFragPosition;
        float distance = length(L);
        L /= distance; // normalize

        float range = light.position.w;

        // Hard cutoff by range (same as point lights)
        if (distance > range)
            continue;

        // ---- Distance attenuation (same style as point lights) ----
        float invDist2 = 1.0 / (distance * distance);

        float x = clamp(distance / range, 0.0, 1.0);
        float rangeFalloff = 1.0 - x * x;   // quadratic
        rangeFalloff *= rangeFalloff;       // ^4 for smoother edge

        float attenuation = invDist2 * rangeFalloff;

        // ---- Cone attenuation using direction.w as cutoff angle ----

        // Light's forward direction (where the cone points)
        vec3 lightDir = normalize(light.direction.xyz);

        // We want the angle between light forward and the vector
        // from light to fragment (opposite of L).
        vec3 L_lightToFrag = -L; // now points from light -> fragment

        float cosTheta = dot(lightDir, L_lightToFrag);

        // Outer cutoff in radians from .w
        float outerAngle = light.direction.w;
        float cosOuter   = cos(outerAngle);

        // Make a slightly smaller inner angle for smooth edge
        // (e.g. 90% of the outer angle)
        float innerAngle = outerAngle * 0.9;
        float cosInner   = cos(innerAngle);

        // If outside the outer cone, no contribution
        if (cosTheta <= cosOuter)
            continue;

        // Smoothstep-style interpolation between inner and outer cone
        float spotFactor = clamp((cosTheta - cosOuter) / (cosInner - cosOuter), 0.0, 1.0);

        attenuation *= spotFactor;

        if (attenuation <= 0.0)
            continue;

        // ---- Final radiance and BRDF evaluation ----
        vec3 lightColor = light.color.rgb;
        float intensity = light.color.a;

        vec3 radiance = lightColor * intensity * attenuation;

        vec3 brdf = EvaluateBRDF(N, V, L, albedo, metallic, roughness, F0);

        Lo += brdf * radiance;
    }

    return Lo;
}

// =========================
// Main PBR path
// =========================

void PBR_Main()
{
    float alpha = texture(albedoTexture, inUV).a;
    if (alpha < 0.5)
    {
        discard;
        return;
    }

    vec3 mrt = texture(metallicRoughnessTexture, inUV).xyz;
    float metallic  = mrt.r * material.metallic_roughness_AO_emission.r;
    float roughness = mrt.g * material.metallic_roughness_AO_emission.b;
    float ao        = texture(aoTexture, inUV).r * material.metallic_roughness_AO_emission.z;
    vec3 emission   = texture(emissiveTexture, inUV).rgb * material.metallic_roughness_AO_emission.a;

    vec3 N = calculateNormal(texture(normalTexture, inUV).xyz * 2.0 - 1.0); // Normal
    vec3 V = normalize(sceneData.camPosition.xyz - inFragPosition);         // viewDirection

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 albedo = ALBEDO;
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Directional and point lights
    vec3 Lo = vec3(0); 
	Lo += AccumulateDirectionalLights(N, V, albedo, metallic, roughness, F0, inFragPosition);
    Lo += AccumulatePointLights(N, V, albedo, metallic, roughness, F0);
	Lo += AccumulateSpotLights(N, V, albedo, metallic, roughness, F0);

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.0) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    outFragColor = vec4(color + emission, 1.0);
    //outFragColor = vec4(texture(shadowMap, inUV).xyz, 1);
}

void main()
{
    PBR_Main();
}
