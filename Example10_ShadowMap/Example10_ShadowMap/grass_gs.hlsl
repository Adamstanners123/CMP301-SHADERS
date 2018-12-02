cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer GrassSizeBuffer
{
	float grassHeight;
	float grassWidth;
	float2 padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float clip : SV_ClipDistance0;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float clip : SV_ClipDistance0;
	float3 worldPosition : TEXCOORD1;
};


[maxvertexcount(12)]
void main(point InputType input[1], inout TriangleStream< OutputType > outputTri)
{
	OutputType output;

	float slope = 1.0f - input[0].normal.y;

	if ((slope <= 0.02f) && (input[0].position.y > 3) && (input[0].position.y < 6))
	{
		// Create first quad in x direction
		float4 perpAngle = float4(1, 0, 0, 0);
		float4 perpTangent = (0, 0, 1, 0);

		float4 norm = float4(input[0].normal.x, input[0].normal.y, input[0].normal.z, 0.f);
		float3 grassFaceNormal = norm.xyz;
		
		// Get first two grass vertices, push second vertex out in direction of normal
		float4 grassVertexOne = input[0].position;
		float4 grassVertexTwo = input[0].position + norm * grassHeight;

		for (int i = 0; i < 3; i++)
		{
			if (i == 1)
			{
				// Create second quad in z direciton
				perpAngle = float4(0, 0, 1, 0);

				// Calculate new face normal
				grassFaceNormal = norm.xyz;
			}
			else if (i == 2)
			{
				// Create third quad in x/z direciton
				perpAngle = float4(0.5, 0, 0.5, 0);

				// Calculate new face normal
				grassFaceNormal = norm.xyz;
			}

			for (int j = 0; j < 1; j++)
			{
				// Move the vertex away from the point position
				output.position = grassVertexOne + perpAngle * grassWidth * grassHeight;
				output.position = mul(output.position, worldMatrix);
				output.position = mul(output.position, viewMatrix);
				output.position = mul(output.position, projectionMatrix);
				output.tex = float2(1, 1);
				output.normal = mul(grassFaceNormal, (float3x3) worldMatrix);
				output.normal = normalize(output.normal);
				output.clip = input[0].clip;
				output.worldPosition = mul(input[0].position, (float3x3)worldMatrix);
				outputTri.Append(output);

				// Move the vertex away from the point position
				output.position = grassVertexOne - perpAngle * grassWidth * grassHeight;
				output.position = mul(output.position, worldMatrix);
				output.position = mul(output.position, viewMatrix);
				output.position = mul(output.position, projectionMatrix);
				output.tex = float2(0, 1);
				output.normal = mul(grassFaceNormal, (float3x3) worldMatrix);
				output.normal = normalize(output.normal);
				output.clip = input[0].clip;
				output.worldPosition = mul(input[0].position, (float3x3)worldMatrix);
				outputTri.Append(output);

				// Move the vertex away from the point position
				output.position = grassVertexTwo + perpAngle * grassWidth * grassHeight;
				output.position = mul(output.position, worldMatrix);
				output.position = mul(output.position, viewMatrix);
				output.position = mul(output.position, projectionMatrix);
				output.tex = float2(1, 0);
				output.normal = mul(grassFaceNormal, (float3x3) worldMatrix);
				output.normal = normalize(output.normal);
				output.clip = input[0].clip;
				output.worldPosition = mul(input[0].position, (float3x3)worldMatrix);
				outputTri.Append(output);

				// Move the vertex away from the point position
				output.position = grassVertexTwo - perpAngle * grassWidth * grassHeight;
				output.position = mul(output.position, worldMatrix);
				output.position = mul(output.position, viewMatrix);
				output.position = mul(output.position, projectionMatrix);
				output.tex = float2(0, 0);
				output.normal = mul(grassFaceNormal, (float3x3) worldMatrix);
				output.normal = normalize(output.normal);
				output.clip = input[0].clip;
				output.worldPosition = mul(input[0].position, (float3x3)worldMatrix);
				outputTri.Append(output);
			}
			outputTri.RestartStrip();
		}
	}
}