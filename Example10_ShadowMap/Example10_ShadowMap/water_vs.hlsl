cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix reflectionMatrix;
};

cbuffer WaterDataBuffer : register(b1)
{
	float3 camPos;
	float2 normRepeat;
	float2 padding;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float4 refractPos : TEXCOORD0;
	float4 reflectPos : TEXCOORD1;
	float3 viewVector : TEXCOORD2;
	float2 tex1 : TEXCOORD3;
	float2 tex2 : TEXCOORD4;
	
};

OutputType main(InputType input)
{
	OutputType output;

	matrix reflectProjectWorld;
	matrix viewProjectWorld;
	float4 worldPosition;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Create the reflection projection world matrix.
	reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
	reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

	// Calculate the input position against the reflectProjectWorld matrix.
	output.reflectPos = mul(input.position, reflectProjectWorld);

	// Create the view projection world matrix for refraction.
	viewProjectWorld = mul(viewMatrix, projectionMatrix);
	viewProjectWorld = mul(worldMatrix, viewProjectWorld);

	// Calculate the input position against the viewProjectWorld matrix.
	output.refractPos = mul(input.position, viewProjectWorld);
	
	// Calculate the position of the vertex in the world.
	worldPosition = mul(input.position, worldMatrix);

	// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
	output.viewVector = camPos.xyz - worldPosition.xyz;

	// Normalize the viewing direction vector.
	output.viewVector = normalize(output.viewVector);
	
	// Create two different texture sample coordinates for tiling the water normal map over the water quad multiple times.
	output.tex1 = input.tex / normRepeat.x;
	output.tex2 = input.tex / normRepeat.y;

	return output;
}