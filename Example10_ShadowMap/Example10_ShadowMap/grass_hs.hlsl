struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float grassTessFactor : TEXCOORD1;
	float clip : SV_ClipDistance0;
};

struct ConstantOutputType
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float clip : SV_ClipDistance0;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
	ConstantOutputType output;


	// Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = 0.5f * (inputPatch[1].grassTessFactor + inputPatch[2].grassTessFactor);		// tessellationFactor;
	output.edges[1] = 0.5f * (inputPatch[2].grassTessFactor + inputPatch[0].grassTessFactor);		// tessellationFactor;
	output.edges[2] = 0.5f * (inputPatch[0].grassTessFactor + inputPatch[1].grassTessFactor);		// tessellationFactor;

																										// Set the tessellation factor for tessallating inside the triangle.
	output.inside = output.edges[1];// tessellationFactor;

	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	OutputType output;

	// Set the position for this control point as the output position.
	output.position = patch[pointId].position;

	output.position.w = 1.f;

	// Set the input colour as the output colour.
	output.tex = patch[pointId].tex;

	// Set the input colour as the output colour.
	output.normal = patch[pointId].normal;

	output.clip = patch[pointId].clip;

	return output;
}
