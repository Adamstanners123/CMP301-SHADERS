// Tessellation vertex shader.
// Doesn't do much, could manipulate the control points
// Pass forward data, strip out some values not required for example.

struct InputType
{
	float4 position : POSITION0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : POSITION0;
	float4 colour : COLOR;
};

OutputType main(InputType input)
{
	OutputType output;

	// Pass the vertex position into the hull shader.
	output.position = input.position;

	// Pass the input color into the hull shader.
	output.colour = float4(1.0, 0.0, 0.0, 1.0);

	return output;
}