Texture2D heightMap : register(t0);
SamplerState Sampler : register(s1);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer TessVars : register(b1)
{
	int maxTessellationDist;
	int minTessellationDist;
	int minTessellationFactor;
	int maxTessellationFactor;
	float3 cameraPosition;
	float padding;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 camPos : TEXCOORD1;
	float tessellationFactor : TEXCOORD2;
	float3 worldPosition : TEXCOORD3;
};

OutputType main(InputType input)
{
	OutputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = input.position;

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and then normalize the final value.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	//output.normal = normalize(output.normal);

	// Calculate the tangent vector against the world matrix only and then normalize the final value.
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	//output.tangent = normalize(output.tangent);

	// Calculate the binormal vector against the world matrix only and then normalize the final value.
	output.binormal = mul(input.binormal, (float3x3)worldMatrix);
	//output.binormal = normalize(output.binormal);
	
	output.camPos = cameraPosition;

	// Calculate the position of the vertex in the world.
	output.worldPosition = mul(input.position, (float3x3)worldMatrix).xyz;

	float viewDistance = distance(output.position.xyz, cameraPosition.xyz);

	float tessDist = saturate((minTessellationDist - viewDistance) / (minTessellationDist - maxTessellationDist));

	// Scale value to a 0-1 range
	output.tessellationFactor = minTessellationFactor + tessDist * (maxTessellationFactor - minTessellationFactor);


	return output;
}