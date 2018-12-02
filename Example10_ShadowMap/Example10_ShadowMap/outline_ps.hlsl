// Texture and sampler registers
Texture2D shaderTexture : register(t0);
SamplerState outlineSampler : register(s0);
SamplerState textureSampler : register(s1);

cbuffer screen : register(b0)
{
	float screenHeight;
	float screenWidth;
	float2 padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 depthPosition : TEXCOORD1;
	float edge_width : TEXCOORD2;
	float edge_intensity : TEXCOORD3;
	float normal_threshold : TEXCOORD4;
	float depth_threshold : TEXCOORD5;
	float normal_sensitivity : TEXCOORD6;
	float depth_sensitivity : TEXCOORD7;
};

float4 main(InputType input) : SV_TARGET
{
	//float edgeWidth = 1.0f;
	//float edgeIntensity = 10.f;
	//float normalThreshold = 0.8f;
	//float depthThreshold = 0.3f;
	//float normalSensitivity = 1.f;
	//float depthSensitivity = 10.f;

	float3 textureColour = shaderTexture.Sample(textureSampler, input.tex);

	float2 edgeOffset = input.edge_width / float2(screenWidth, screenHeight);

	float4 n1 = shaderTexture.Sample(outlineSampler, input.tex + float2(-1, -1) * edgeOffset);

	float4 n2 = shaderTexture.Sample(outlineSampler, input.tex + float2(1, 1) * edgeOffset);

	float4 n3 = shaderTexture.Sample(outlineSampler, input.tex + float2(-1, 1) * edgeOffset);

	float4 n4 = shaderTexture.Sample(outlineSampler, input.tex + float2(1, -1) * edgeOffset);

	float4 diagonalDelta = abs(n1 - n2) + abs(n3 - n4);

	float normalDelta = dot(diagonalDelta.xyz, 1.f);

	float depthDelta = diagonalDelta.w;

	normalDelta = saturate((normalDelta - input.normal_threshold) * input.normal_sensitivity);

	depthDelta = saturate((depthDelta - input.depth_threshold) * input.depth_sensitivity);

	float edgeAmount = saturate(normalDelta + depthDelta) * input.edge_intensity;
	
	textureColour *= (1.f - edgeAmount);

	return float4(textureColour, 1.f);
}
