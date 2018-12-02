cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer TessVars : register(b1)
{
	int grassTessFactor;
	float3 cameraPosition;
	float4 grassClipPlane;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float grassTessFactor : TEXCOORD1;
	float clip : SV_ClipDistance0;
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
	output.normal = normalize(output.normal);

	// Scale value to a 0-1 range
	output.grassTessFactor = grassTessFactor;

	// Set the clipping plane
	output.clip = dot(mul(input.position, worldMatrix), grassClipPlane);

	return output;
}