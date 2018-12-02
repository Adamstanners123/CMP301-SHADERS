Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};

cbuffer MeshTypeBuffer : register(b1)
{
	float2 meshType;
	float width;
	float height;
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
    float4 lightViewPos : TEXCOORD1;
};

float3 getNormal(float2 tex)
{
	float texelSize = 1.f / float2(width, height);

	float2 offsets[4] =
	{
		(tex + float2(-texelSize, 0)),
		(tex + float2(texelSize,  0)),
		(tex + float2(0, -texelSize)),
		(tex + float2(0,  texelSize))
	};

	float pixels[4];
	for (int i = 0; i < 4; i++)
	{
		pixels[i] = texture0.SampleLevel(sampler0, tex, offsets[i]).x;
	}

	float2 step = float2(1.0, 0.0);

	float3 vertexA = normalize(float3(step.xy, pixels[1] - pixels[0]));
	float3 vertexB = normalize(float3(step.yx, pixels[3] - pixels[2]));

	return cross(vertexA, vertexB).rbg;
}

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);

	if (meshType.x == 1.f)
	{
		// Sample pixel color at each texel 
		float height = texture0.SampleLevel(sampler0, input.position.xz*0.01, 0);

		output.normal = getNormal(input.tex);

		// Set height equal to colour multiplied by 16
		output.position.y = height * 16;
	}

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Calculate the position of the vertice as viewed by the light source.
	output.lightViewPos = mul(input.position, worldMatrix);
	output.lightViewPos = mul(output.lightViewPos, lightViewMatrix);
	output.lightViewPos = mul(output.lightViewPos, lightProjectionMatrix);

	output.tex = input.tex;

	//if (meshType.y = 0.f)
	//{
		output.normal = mul(input.normal, (float3x3)worldMatrix);
		output.normal = normalize(output.normal);
	//}

	return output;
}