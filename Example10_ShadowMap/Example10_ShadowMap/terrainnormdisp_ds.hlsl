// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
Texture2D heightMap : register(t0);

SamplerState Sampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct ConstantOutputType
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 camPos : TEXCOORD1;
	float3 worldPosition : TEXCOORD2;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 camPos : TEXCOORD1;
	float3 worldPosition : TEXCOORD2;
};

[domain("tri")]
OutputType main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 3> patch)
{
	OutputType output;

	// Interpolate patch attributes to generated vertices.
	output.position      = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;
	
	output.tex           = uvwCoord.x * patch[0].tex      + uvwCoord.y * patch[1].tex      + uvwCoord.z * patch[2].tex;
	
	output.normal        = uvwCoord.x * patch[0].normal   + uvwCoord.y * patch[1].normal   + uvwCoord.z * patch[2].normal;

	output.tangent       = uvwCoord.x * patch[0].tangent  + uvwCoord.y * patch[1].tangent  + uvwCoord.z * patch[2].tangent;

	output.binormal      = uvwCoord.x * patch[0].binormal + uvwCoord.y * patch[1].binormal + uvwCoord.z * patch[2].binormal;

	output.camPos		 = patch[0].camPos;

	output.worldPosition = uvwCoord.x * patch[0].worldPosition + uvwCoord.y * patch[1].worldPosition + uvwCoord.z * patch[2].worldPosition;

	output.normal        = normalize(output.normal);
	output.tangent		 = normalize(output.tangent);
	output.binormal	     = normalize(output.binormal);
	
	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	float slope = 1.0f - output.normal.y;

	if (slope >= 0.05)
	{
		const float MipInterval = 10.f;
		float mipLevel = clamp((distance(output.position, output.camPos) - MipInterval) / MipInterval, 0.0f, 5.f);

		// Sample height map (stored in alpha channel).
		float height = heightMap.SampleLevel(Sampler, output.tex, mipLevel);

		output.position.xyz += (height) * output.normal;

	}

	output.position.w = 1.0f;

	// Calculate the position of the new vertex against the world, view, and projection matrices.
	output.position = mul(output.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	return output;
}