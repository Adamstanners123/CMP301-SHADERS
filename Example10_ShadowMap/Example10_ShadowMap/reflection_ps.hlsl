Texture2D objTexture : register(t0);
Texture2D normalTexture : register(t1);

// Max number of lights
#define MAX_LIGHTS 3

// Light Types
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

SamplerState texSampler  : register(s0);

struct Light
{
	float4 ambient;
	float4 diffuse;
	float3 position;
	float spotAngle;
	float3 direction;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	int lightType;
	bool enabled;
};

cbuffer LightBuffer : register(b0)
{
	Light Lights[MAX_LIGHTS];
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float clip : SV_ClipDistance0;
};

float4 main(InputType input) : SV_TARGET
{
	float4 textureColor;
	float4 bumpMap;
	float3 bumpNormal;
	float lightIntensity;
	float4 finalColour;

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = objTexture.Sample(texSampler, input.tex);

	// Calculate the amount of light on this pixel using the normal map.
	bumpMap = normalTexture.Sample(texSampler, input.tex);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	bumpNormal = input.normal + bumpMap.x * input.tangent + bumpMap.y * input.binormal;
	bumpNormal = normalize(bumpNormal);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	finalColour = Lights[2].diffuse;
			
	// Multiply the texture pixel and the final diffuse color to get the final pixel color result.
	finalColour = finalColour * textureColor;

	return finalColour;
}