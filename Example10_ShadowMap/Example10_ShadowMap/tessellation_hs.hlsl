// Tessellation Hull Shader
// Prepares control points for tessellation
cbuffer TessFactors : register (b0)
{
	float tessFactor1;
	float tessFactor2;
	float tessFactor3;
	float tessFactor4;
	float tessFactor5;
	float tessFactor6;
};

struct InputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;


    // Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = tessFactor1;// tessellationFactor;
	output.edges[1] = tessFactor2;//tessellationFactor;
	output.edges[2] = tessFactor3;// tessellationFactor;
	output.edges[3] = tessFactor4;// tessellationFactor;

    // Set the tessellation factor for tessallating inside the triangle.
	output.inside[0] = tessFactor5;// tessellationFactor;
	output.inside[1] = tessFactor6;// tessellationFactor;

    return output;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;


    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;

    // Set the input colour as the output colour.
    output.colour = patch[pointId].colour;

    return output;
}