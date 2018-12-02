// Tessellation Hull Shader
// Prepares control points for tessellation
struct InputType
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
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 camPos : TEXCOORD1;
	float3 worldPosition : TEXCOORD2;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
	ConstantOutputType output;


	// Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = 0.5f * (inputPatch[1].tessellationFactor + inputPatch[2].tessellationFactor);		// tessellationFactor;
	output.edges[1] = 0.5f * (inputPatch[2].tessellationFactor + inputPatch[0].tessellationFactor);		// tessellationFactor;
	output.edges[2] = 0.5f * (inputPatch[0].tessellationFactor + inputPatch[1].tessellationFactor);		// tessellationFactor;

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

	// Set the input colour as the output colour.
	output.tex = patch[pointId].tex;

	// Set the input colour as the output colour.
	output.normal = patch[pointId].normal;

	// Set the input colour as the output colour.
	output.tangent = patch[pointId].tangent;

	// Set the input colour as the output colour.
	output.binormal = patch[pointId].binormal;

	output.camPos = patch[pointId].camPos;

	output.worldPosition = patch[pointId].worldPosition;

	return output;
}
