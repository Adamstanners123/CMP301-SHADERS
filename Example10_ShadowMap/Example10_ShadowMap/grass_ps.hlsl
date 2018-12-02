// Max number of lights
#define MAX_LIGHTS 3

// Light Types
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

Texture2D objTexture : register(t0);

SamplerState textureSampler : register(s0);

struct Light
{
	float4 ambient;
	float4 diffuse;
	float3 position;
	float spotAngle;
	float3 direction;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	int lightType;
	float enabled;
};

cbuffer LightBuffer : register(b0)
{
	Light Lights[MAX_LIGHTS];
};

cbuffer NormalBuffer : register(b1)
{
	bool showNormals;
	float3 padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float clip : SV_ClipDistance0;
	float3 worldPosition : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 ldiffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(ldiffuse * intensity);
	return colour;
}

// Calculate attenuation
float calculateAttenuation(float constantA, float linearA, float quadraticA, float3 lightVector)
{
	float distance = length(lightVector);

	return 1.0f / (constantA + linearA * distance + quadraticA * distance * distance);
}

// Calculate spot cone
float calculateSpotCone(float spotAngle, float3 lightDirection, float3 lightVector)
{
	float minCos = cos(spotAngle);
	float maxCos = (minCos + 1.0f) / 2.0f;
	float cosAngle = dot(lightDirection, -lightVector);

	return smoothstep(minCos, maxCos, cosAngle);

}

// Calculate directional light
float4 calculateDirectionalLight(Light light, float3 normal)
{
	float4 colour;

	colour = calculateLighting(light.direction, normal, light.diffuse);

	return colour + light.ambient;
}

// Calculate point light
float4 calculatePointLight(Light light, float3 lightVector, float3 normal)
{
	float4 colour;

	float attenuation = calculateAttenuation(light.constantAttenuation, light.linearAttenuation, light.quadraticAttenuation, lightVector);

	colour = calculateLighting(lightVector, normal, light.diffuse);

	return colour + light.ambient;
}

// Calculate spot light
float4 calculateSpotLight(Light light, float3 lightVector, float3 normal)
{
	float attenuation = calculateAttenuation(light.constantAttenuation, light.linearAttenuation, light.quadraticAttenuation, lightVector);
	float spotIntensity = calculateSpotCone(light.spotAngle, light.direction, lightVector);

	float4 colour = calculateLighting(lightVector, normal, light.diffuse) * attenuation * spotIntensity;

	return colour + light.ambient;
}

float4 doLighting(float3 normal, float3 worldPosition)
{
	// Declare light vector and colour.
	float4 finalColour = { 0, 0, 0, 0 };
	float3 lightVector = { 0, 0, 0 };

	// Loop through lights array and determine pixel colour
	[unroll]
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (Lights[i].enabled < 0.5f) continue;

		switch (Lights[i].lightType)
		{
		case DIRECTIONAL_LIGHT:
		{
			finalColour += calculateDirectionalLight(Lights[i], normal);				// Calculate directional light
			break;
		}
		case POINT_LIGHT:
		{
			lightVector = normalize(Lights[i].position - worldPosition);				// Calculate point light
			finalColour += calculatePointLight(Lights[i], lightVector, normal);
			break;
		}
		case SPOT_LIGHT:
		{
			lightVector = normalize(Lights[i].position - worldPosition);				// Calculate spot light
			finalColour += calculateSpotLight(Lights[i], lightVector, normal);
			break;
		}

		}

	}

	return finalColour;

}

float4 main(InputType input) : SV_TARGET
{
	float4 lightColour = {0.f, 0.f, 0.f, 0.f };

	lightColour = doLighting(input.normal, input.worldPosition);

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	float4 textureColor = objTexture.Sample(textureSampler, input.tex);

	if (textureColor.a < 0.1)
	{
		clip(textureColor.a - 0.1);
	}
	

	return textureColor * lightColour;
}