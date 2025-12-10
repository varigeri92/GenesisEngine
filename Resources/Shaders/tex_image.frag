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
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

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

void PBR_Main(){
	float alpha = texture(albedoTexture,inUV).a;
	if(alpha < 0.5){
		discard;
		return;
	}

	vec3 mrt = texture(metallicRoughnessTexture,inUV).xyz;
	float metallic = mrt.r * material.metallic_roughness_AO_emission.r;
	float roughness = mrt.g * material.metallic_roughness_AO_emission.b;
	float ao = texture(aoTexture, inUV).r * material.metallic_roughness_AO_emission.z;
	vec3 emission = texture(emissiveTexture, inUV).rgb * material.metallic_roughness_AO_emission.a;
	
	
	vec3 N = calculateNormal(texture(normalTexture, inUV).xyz * 2.0 - 1.0); //Normal
	vec3 V = normalize(sceneData.camPosition.xyz - inFragPosition); //viewDirection

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
	vec3 albedo = ALBEDO;
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// reflectance equation
	vec3 Lo = vec3(0.0);
	for(int i=0;i<sceneData.pointLight_count;++i)
	{
		// calculate per-light radiance
		vec3 L = normalize(pointLights.objects[i].position.xyz - inFragPosition);
		vec3 H = normalize(V + L);
		float distance = length(pointLights.objects[i].position.xyz - inFragPosition);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = pointLights.objects[i].color.xyz * attenuation * pointLights.objects[i].color.w;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G   = GeometrySmith(N, V, L, roughness);
		vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
		vec3 specular = numerator / denominator;

		// kS is equal to Fresnel
		vec3 kS = F;
		// for energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS.
		vec3 kD = vec3(1.0) - kS;
		// multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light).
		kD *= 1.0 - metallic;

		// scale light by NdotL
		float NdotL = max(dot(N, L), 0.0);

		// add to outgoing radiance Lo
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}

	// ambient lighting (note that the next IBL tutorial will replace 
	// this ambient lighting with environment lighting).
	vec3 ambient = vec3(0.03) * albedo * ao;

	vec3 color = ambient + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// gamma correct
	color = pow(color, vec3(1.0/2.2));

	outFragColor = vec4(color + emission, 1.0);
}
void main(){
	PBR_Main();
}