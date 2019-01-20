#pragma Parameter cbuf			Material
#pragma Parameter rgb			color			(1, 1, 1)
#pragma Parameter float			alpha			1
#pragma Parameter rgb			emission		(0, 0, 0)
#pragma Parameter float(0,1)	smoothness		1
#pragma Parameter float(0,1)	metallic		1
#pragma Parameter float(0,10)	envIntensity	1.0
#pragma Parameter srv			EnvironmentMap

#define RootSigPBR "CBV(b3, visibility=SHADER_VISIBILITY_PIXEL),"\
"DescriptorTable(SRV(t2)),"\
"StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_LINEAR, visibility=SHADER_VISIBILITY_PIXEL),"

struct MaterialBuffer {
	float3 baseColor;
	float alpha;
	float3 emission;
	float smoothness;
	float metallic;
	float environmentIntensity;
};
ConstantBuffer<MaterialBuffer> Material : register(b3);

TextureCube<float3> EnvironmentMap : register(t2);
sampler Sampler : register(s0);

#define PI 3.14159265359
#define INV_PI 0.31830988618

// Unity BRDF from Unity built-in shaders

float Pow5(float x) {
	float x2 = x * x;
	return x2 * x2 * x;
}

float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float perceptualRoughness) {
	float fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
	// Two schlick fresnel term
	float lightScatter = (1 + (fd90 - 1) * Pow5(1 - NdotL));
	float viewScatter = (1 + (fd90 - 1) * Pow5(1 - NdotV));

	return lightScatter * viewScatter;
}
float SmithJointGGXVisibilityTerm(float NdotL, float NdotV, float roughness) {
	float a = roughness;
	float lambdaV = NdotL * (NdotV * (1 - a) + a);
	float lambdaL = NdotV * (NdotL * (1 - a) + a);

	return 0.5f / (lambdaV + lambdaL + 1e-5f);
}

float GGXTerm(float NdotH, float roughness) {
	float a2 = roughness * roughness;
	float d = (NdotH * a2 - NdotH) * NdotH + 1.0f;
	return INV_PI * a2 / (d * d + 1e-7f);
}
float3 FresnelTerm(float3 F0, float cosA) {
	float t = Pow5(1 - cosA);   // ala Schlick interpoliation
	return F0 + (1 - F0) * t;
}
float3 FresnelLerp(float3 F0, float3 F90, float cosA) {
	float t = Pow5(1 - cosA);   // ala Schlick interpoliation
	return lerp(F0, F90, t);
}

#define COLORSPACE_GAMMA

#ifdef COLORSPACE_GAMMA
#define unity_ColorSpaceDielectricSpec float4(0.220916301, 0.220916301, 0.220916301, 1.0 - 0.220916301)
#else // Linear values
#define unity_ColorSpaceDielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04) // standard dielectric reflectivity coef at incident angle (= 4%)
#endif

float OneMinusReflectivityFromMetallic(float metallic) {
	// We'll need oneMinusReflectivity, so
	//   1-reflectivity = 1-lerp(dielectricSpec, 1, metallic) = lerp(1-dielectricSpec, 0, metallic)
	// store (1-dielectricSpec) in unity_ColorSpaceDielectricSpec.a, then
	//   1-reflectivity = lerp(alpha, 0, metallic) = alpha + metallic*(0 - alpha) =
	//                  = alpha - metallic * alpha
	float oneMinusDielectricSpec = unity_ColorSpaceDielectricSpec.a;
	return oneMinusDielectricSpec - metallic * oneMinusDielectricSpec;
}
float3 DiffuseAndSpecularFromMetallic(float3 albedo, float metallic, out float3 specColor, out float oneMinusReflectivity) {
	specColor = lerp(unity_ColorSpaceDielectricSpec.rgb, albedo, metallic);
	oneMinusReflectivity = OneMinusReflectivityFromMetallic(metallic);
	return albedo * oneMinusReflectivity;
}

float3 UnityBRDF(float3 albedo, float perceptualRoughness, float metallic, float3 normal, float3 viewDir, float3 lightDir, float3 lightColor) {
	float oneMinusReflectivity;
	float3 specColor;
	float3 diffColor = DiffuseAndSpecularFromMetallic(albedo, metallic, specColor, oneMinusReflectivity);

	float3 halfDir = normalize(float3(lightDir) + viewDir);

	// The amount we shift the normal toward the view vector is defined by the dot product.
	float shiftAmount = dot(normal, viewDir);
	normal = shiftAmount < 0.0f ? normal + viewDir * (-shiftAmount + 1e-5f) : normal;
	// A re-normalization should be applied here but as the shift is small we don't do it to save ALU.
	//normal = normalize(normal);

	float nv = dot(normal, viewDir);

	float nl = saturate(dot(normal, lightDir));
	float nh = saturate(dot(normal, halfDir));

	float lv = saturate(dot(lightDir, viewDir));
	float lh = saturate(dot(lightDir, halfDir));

	// Diffuse term
	float diffuseTerm = DisneyDiffuse(nv, nl, lh, perceptualRoughness) * nl / PI;

	float roughness = perceptualRoughness * perceptualRoughness;

	// GGX with roughtness to 0 would mean no specular at all, using max(roughness, 0.002) here to match HDrenderloop roughtness remapping.
	roughness = max(roughness, 0.002);
	float V = SmithJointGGXVisibilityTerm(nl, nv, roughness);
	float D = GGXTerm(nh, roughness);

	float specularTerm = V * D; // Torrance-Sparrow model, Fresnel is applied later

	#ifdef COLORSPACE_GAMMA
	specularTerm = sqrt(max(1e-4h, specularTerm));
	#endif

	// specularTerm * nl can be NaN on Metal in some cases, use max() to make sure it's a sane value
	specularTerm = max(0, specularTerm * nl);

	// To provide true Lambert lighting, we need to be able to kill specular completely.
	specularTerm *= any(specColor) ? 1.0 : 0.0;
	half grazingTerm = saturate(1.0 - perceptualRoughness + 1.0 - oneMinusReflectivity);
	return diffColor * (lightColor * diffuseTerm) +
		specularTerm * lightColor * FresnelTerm(specColor, lh);
}
float3 UnityGI(float3 albedo, float perceptualRoughness, float metallic, float3 normal, float3 viewDir, float3 giSpecular) {
	float oneMinusReflectivity;
	float3 specColor;
	float3 diffColor = DiffuseAndSpecularFromMetallic(albedo, metallic, specColor, oneMinusReflectivity);

	// The amount we shift the normal toward the view vector is defined by the dot product.
	float shiftAmount = dot(normal, viewDir);
	normal = shiftAmount < 0.0f ? normal + viewDir * (-shiftAmount + 1e-5f) : normal;
	// A re-normalization should be applied here but as the shift is small we don't do it to save ALU.
	//normal = normalize(normal);

	float nv = dot(normal, viewDir);

	float roughness = perceptualRoughness * perceptualRoughness;

	// GGX with roughtness to 0 would mean no specular at all, using max(roughness, 0.002) here to match HDrenderloop roughtness remapping.
	roughness = max(roughness, 0.002);

	// surfaceReduction = Int D(NdotH) * NdotH * Id(NdotL>0) dH = 1/(roughness^2+1)
	float surfaceReduction;
	#ifdef COLORSPACE_GAMMA
	surfaceReduction = 1.0 - 0.28*roughness*perceptualRoughness;      // 1-0.28*x^3 as approximation for (1/(x^4+1))^(1/2.2) on the domain [0;1]
	#else
	surfaceReduction = 1.0 / (roughness*roughness + 1.0);           // fade \in [0.5;1]
	#endif

	surfaceReduction = 1.0 - roughness * perceptualRoughness*surfaceReduction;

	// To provide true Lambert lighting, we need to be able to kill specular completely.
	half grazingTerm = saturate(1.0 - perceptualRoughness + 1.0 - oneMinusReflectivity);
	return surfaceReduction * giSpecular * FresnelLerp(specColor, grazingTerm, nv);
}

float3 GlossyEnvironment(float perceptualRoughness, float3 refl) {
	return EnvironmentMap.SampleLevel(Sampler, refl, perceptualRoughness * (1.7 - 0.7*perceptualRoughness) * 11) * Material.environmentIntensity;
}

struct Surface {
	float3 albedo;
	float metallic;
	float smoothness;
	float3 normal;
	float3 worldPos;
};
float LightDistanceAttenuation(float x, float r) {
	float atten = 1 / ((x + 1) * (x + 1));
	float b = 1 / ((r + 1) * (r + 1));
	return (atten - b) / (1 - b);
}
float LightAngleAttenuation(float ldp, float r) {
	return saturate(10 * (max(0, ldp) - r));
}

float3 ShadeLight(Light light, float3 albedo, float roughness, float metallic, float3 normal, float3 worldPos, float3 view) {
	float3 ldir = light.Direction.xyz;
	float3 lcol = light.Color.rgb;
	if (light.Color.w > 0) { // point or spot light
		ldir = worldPos - light.Position.xyz;
		float dist = max(.0001, length(ldir));
		ldir /= dist;
		lcol *= LightDistanceAttenuation(dist, light.Position.w);
		if (light.Color.w == 1) lcol *= LightAngleAttenuation(dot(ldir, light.Direction.xyz), light.Direction.w); // spot light
	}

	float atten = ShadowAttenuation(worldPos, normal, light);
	return max(0, UnityBRDF(albedo, roughness, metallic, normal, -view, -ldir, lcol)) * atten;
}

float3 ShadePoint(Surface sfc, float4 screenPos, float3 view) {
	uint2 index = GetLightMask(screenPos);

	sfc.albedo *= Material.baseColor;
	sfc.smoothness *= Material.smoothness;
	sfc.metallic *= Material.metallic;

	float roughness = 1.0 - sfc.smoothness;

	float3 giSpecular = 0;
	giSpecular += GlossyEnvironment(roughness, reflect(view, sfc.normal));
	giSpecular += lerp(Lighting.GroundColor.rgb, Lighting.SkyColor.rgb, saturate(sfc.normal.y));

	float3 light = UnityGI(sfc.albedo, roughness, sfc.metallic, sfc.normal, -view, giSpecular);

	for (unsigned int i = 0; i < 64; i++)
		if ((i < 32 && index.x & (1 << i)) || (i >= 32 && (index.y & (1 << (i - 32)))))
			light += ShadeLight(Lighting.Lights[i], sfc.albedo, roughness, sfc.metallic, sfc.normal, sfc.worldPos, view);
	
	light += Material.emission;
	return light;
}