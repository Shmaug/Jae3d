#pragma multi_compile COLORSPACE_GAMMA _
#pragma Parameter cbuf		 Material
#pragma Parameter rgb		 color			(1, 1, 1)
#pragma Parameter float(0,1) roughness		.5
#pragma Parameter float(0,1) metallic		0

struct MaterialBuffer {
	float3 baseColor;
	float roughness;
	float metallic;
};
ConstantBuffer<MaterialBuffer> Material : register(b3);

#define RootSigPBR "CBV(b3, space=0, visibility=SHADER_VISIBILITY_PIXEL),"

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
	// Ref: http://jcgt.org/published/0003/02/03/paper.pdf
#if 0
	// Original formulation:
	//  lambda_v    = (-1 + sqrt(a2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5f;
	//  lambda_l    = (-1 + sqrt(a2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5f;
	//  G           = 1 / (1 + lambda_v + lambda_l);

	// Reorder code to be more optimal
	float a = roughness;
	float a2 = a * a;

	float lambdaV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
	float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);

	// Simplify visibility term: (2.0f * NdotL * NdotV) /  ((4.0f * NdotL * NdotV) * (lambda_v + lambda_l + 1e-5f));
	return 0.5f / (lambdaV + lambdaL + 1e-5f);  // This function is not intended to be running on Mobile,
												// therefore epsilon is smaller than can be represented by float
#else
	// Approximation of the above formulation (simplify the sqrt, not mathematically correct but close enough)
	float a = roughness;
	float lambdaV = NdotL * (NdotV * (1 - a) + a);
	float lambdaL = NdotV * (NdotL * (1 - a) + a);

	return 0.5f / (lambdaV + lambdaL + 1e-5f);
#endif
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
	albedo *= Material.baseColor;
	perceptualRoughness *= Material.roughness;
	metallic *= Material.metallic;

	float oneMinusReflectivity;
	float3 specColor;
	float3 diffColor = DiffuseAndSpecularFromMetallic(albedo, metallic, specColor, oneMinusReflectivity);

	float3 halfDir = normalize(float3(lightDir) + viewDir);

	// The amount we shift the normal toward the view vector is defined by the dot product.
	float shiftAmount = dot(normal, viewDir);
	normal = shiftAmount < 0.0f ? normal + viewDir * (-shiftAmount + 1e-5f) : normal;
	// A re-normalization should be applied here but as the shift is small we don't do it to save ALU.
	//normal = normalize(normal);

	float nv = saturate(dot(normal, viewDir)); // TODO: this saturate should no be necessary here

	float nl = saturate(dot(normal, lightDir));
	float nh = saturate(dot(normal, halfDir));

	float lv = saturate(dot(lightDir, viewDir));
	float lh = saturate(dot(lightDir, halfDir));

	// Diffuse term
	float diffuseTerm = DisneyDiffuse(nv, nl, lh, perceptualRoughness) * nl;

	// Specular term
	// HACK: theoretically we should divide diffuseTerm by Pi and not multiply specularTerm!
	// BUT 1) that will make shader look significantly darker than Legacy ones
	// and 2) on engine side "Non-important" lights have to be divided by Pi too in cases when they are injected into ambient SH
	float roughness = perceptualRoughness * perceptualRoughness;

	// GGX with roughtness to 0 would mean no specular at all, using max(roughness, 0.002) here to match HDrenderloop roughtness remapping.
	roughness = max(roughness, 0.002);
	float V = SmithJointGGXVisibilityTerm(nl, nv, roughness);
	float D = GGXTerm(nh, roughness);

	float specularTerm = V * D * PI; // Torrance-Sparrow model, Fresnel is applied later

#   ifdef COLORSPACE_GAMMA
	specularTerm = sqrt(max(1e-4h, specularTerm));
#   endif

	// specularTerm * nl can be NaN on Metal in some cases, use max() to make sure it's a sane value
	specularTerm = max(0, specularTerm * nl);

	// surfaceReduction = Int D(NdotH) * NdotH * Id(NdotL>0) dH = 1/(roughness^2+1)
	float surfaceReduction;
#   ifdef COLORSPACE_GAMMA
	surfaceReduction = 1.0 - 0.28*roughness*perceptualRoughness;      // 1-0.28*x^3 as approximation for (1/(x^4+1))^(1/2.2) on the domain [0;1]
#   else
	surfaceReduction = 1.0 / (roughness*roughness + 1.0);           // fade \in [0.5;1]
#   endif

	// To provide true Lambert lighting, we need to be able to kill specular completely.
	specularTerm *= any(specColor) ? 1.0 : 0.0;

	float grazingTerm = saturate((1 - perceptualRoughness) + (1 - oneMinusReflectivity));
	float3 color = diffColor * (/*gi.diffuse + */lightColor * diffuseTerm)
		+ specularTerm * lightColor * FresnelTerm(specColor, lh)
		;//+ surfaceReduction * gi.specular * FresnelLerp(specColor, grazingTerm, nv);

	return color;
}