#define PI 3.14159265359f


struct DirectionalLight
{
	float3 colour;
	float ambientStrength;
	
	float3 direction;
	float padding1;
};

struct PointLight
{
	float3 position;
	float  strength;
	float3 colour;

	float  radius;
};

struct SpotLight
{
	float3 position;
	float padding1;

	float3 colour;
	float padding2;

	float attenuationConstant;
	float attenuationLinear;
	float attenuationQuadratic;
	float innerCutoff;

	float outerCutoff;
	float3 direction;
};

struct Material
{
	float shininess;
	float specularity;
	float metallic;
	float roughness;
	float3 albedo;
	float ao;
	//float2 padding1;
};

cbuffer constantBuffer : register(b0)
{
	//PACK_SEAM
	int numPointLights;
	int numSpotLights;
	float2 padding1;

	//PACK_SEAM
	int useNormalMapping;
	int useParallaxOcclusionMapping;
	float parallaxOcclusionMappingHeight;
	int fresnel;

	//PACK_SEAM
	Material objectMaterial;

	//PACK_SEAM
	DirectionalLight directionalLight;
	PointLight pointLights[10];
	SpotLight spotLights[20];

	//PACK_SEAM
	float3 cameraPosition;
	float padding2;

	
};

cbuffer constantBuffer : register(b1)
{
	float4 cbcolor;
	float cbroughness;
	float cbmetallic;
	float cbdissolveThreshold;
	float cblineWidth;
	float4 cbdissolveColor;
}

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent: TANGENT;
	float3 worldPos : WORLD_POSIITION;
};

Texture2D albedoSRV : TEXTURE : register(t0);
Texture2D metallicSRV : TEXTURE: register(t1);//specularTexture
Texture2D normalSRV : TEXTURE: register(t2);
Texture2D roughSRV : TEXTURE: register(t3);//depthTexture

Texture2D brdfLUT : register(t4);
TextureCube skyIR : register(t5);
TextureCube skyPrefilter: register(t6);
Texture2D dissolveNoiseTexture : TEXTURE: register(t7);

SamplerState basicSampler : SAMPLER : register(s0);


float NormalDistributionGGXTR(float3 normalVec, float3 halfwayVec, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;   // a2 = a^2
	float NdotH = max(dot(normalVec, halfwayVec), 0.0);     // NdotH = normalVec.halfwayVec
	float NdotH2 = NdotH * NdotH;   // NdotH2 = NdotH^2
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return nom / denom;
}


float GeometrySchlickGGX(float NdotV, float roughness)  // k is a remapping of roughness based on direct lighting or IBL lighting
{
	float r = roughness + 1.0f;
	float k = (r * r) / 8.0f;

	float nom = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 normalVec, float3 viewDir, float3 lightDir, float k)
{
	float NdotV = max(dot(normalVec, viewDir), 0.0f);
	float NdotL = max(dot(normalVec, lightDir), 0.0f);
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)   // cosTheta is n.v and F0 is the base reflectivity
{
	return (F0 + (1.0f - F0) * pow(1.0 - cosTheta, 5.0f));
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)   // cosTheta is n.v and F0 is the base reflectivity
{
	return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0f);
}

void CalcRadiance(PS_INPUT input, float3 viewDir, float3 normalVec, float3 albedo, float roughness, float metallic, float3 lightPos, float3 lightCol, float3 F0, out float3 rad)
{
	//static const float PI = 3.14159265359;

	//calculate light radiance
	float3 lightDir = normalize(lightPos - input.worldPos);
	float3 halfwayVec = normalize(viewDir + lightDir);
	float distance = length(lightPos - input.worldPos);
	float attenuation = 1.0f / (distance * distance);
	float3 radiance = lightCol * attenuation;

	//Cook-Torrance BRDF
	float D = NormalDistributionGGXTR(normalVec, halfwayVec, roughness);
	float G = GeometrySmith(normalVec, viewDir, lightDir, roughness);
	float3 F = FresnelSchlick(max(dot(halfwayVec, viewDir), 0.0f), F0);

	float3 kS = F;
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metallic;

	float3 nom = D * G * F;
	float denom = 4 * max(dot(normalVec, viewDir), 0.0f) * max(dot(normalVec, lightDir), 0.0) + 0.001f; // 0.001f just in case product is 0
	float3 specular = nom / denom;

	//Add to outgoing radiance Lo
	float NdotL = max(dot(normalVec, lightDir), 0.0f);
	rad = (((kD * albedo / PI) + specular) * radiance * NdotL);
}


float4 main(PS_INPUT input) : SV_TARGET
{

	float3 albedocolor = albedoSRV.Sample(basicSampler, input.uv).rgb; //pow(,2.2f)

	float3 noise = dissolveNoiseTexture.Sample(basicSampler, input.uv).rgb;
	clip(noise.r - cbdissolveThreshold);

	//Normal
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	float3 normalFromMap = normalSRV.Sample(basicSampler, input.uv).xyz * 2 - 1;

	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);
	input.normal = normalize(mul(normalFromMap, TBN));

	float3 normalVec = input.normal;

	//Metallic
	float metallic = metallicSRV.Sample(basicSampler, input.uv).r;

	//Rough
	float rough = roughSRV.Sample(basicSampler, input.uv).r;

	float3 viewDir = normalize(cameraPosition - input.worldPos);

	float3 R = reflect(-viewDir, normalVec);

	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedocolor, metallic);

	float3 rad = float3(0.0f, 0.0f, 0.0f);
	//reflectance equation
	float3 Lo = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < numPointLights; i++)
	{
		CalcRadiance(input, viewDir, normalVec, albedocolor, rough, metallic, pointLights[i].position, pointLights[i].colour, F0, rad);
		Lo += rad;
	}

	float3 kS = FresnelSchlickRoughness(max(dot(normalVec, viewDir), 0.0f), F0, rough);
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metallic;

	float3 irradiance = skyIR.Sample(basicSampler, normalVec).rgb;
	float3 diffuse = albedocolor ;// * irradiance

	const float MAX_REF_LOD = 4.0f;
	float3 prefilteredColor = skyPrefilter.SampleLevel(basicSampler, R, rough * MAX_REF_LOD).rgb;
	float2 brdf = brdfLUT.Sample(basicSampler, float2(max(dot(normalVec, viewDir), 0.0f), rough)).rg;
	float3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

	float3 ambient = (kD * diffuse + specular) * objectMaterial.ao;//  
	float3 color = ambient + Lo;

	/*color = color / (color + float3(1.0f, 1.0f, 1.0f));
	color = pow(color, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));*/
	//color = pow(color,2.2f);

	float t = 1 - smoothstep(0.0, cblineWidth, noise.r - cbdissolveThreshold);
	color = lerp(color, cbdissolveColor, t * step(0.001, cbdissolveThreshold));

	return float4(color, 1.0f);
}