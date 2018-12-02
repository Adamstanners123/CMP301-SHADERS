// Max number of lights
#define MAX_LIGHTS 3

// Texture and lighting shader
Texture2D objTexture : register(t0);

SamplerState textureSampler : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPosition : TEXCOORD1;
};

float4 main(InputType input) : SV_TARGET
{
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	float4 textureColor = objTexture.Sample(textureSampler, input.tex);

	return textureColor;
}