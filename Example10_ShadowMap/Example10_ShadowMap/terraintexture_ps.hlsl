// Max number of lights
#define MAX_LIGHTS 3

// Light Types
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// Terrain texture and lighting shader
Texture2D grassTexture : register(t0);
Texture2D rockTexture  : register(t1);

SamplerState texSampler : register(s0);

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
	float4 worldPosition : TEXCOORD1;
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

float4 slopeTexture(float2 tex, float3 normal, float3 worldPosition)
{
	float4 grassColour;
	float4 slopeColour;
	float4 rockColour;
	float4 textureColour;
	float4 diffuseColour;
	float slope;
	float blendAmount;

	// Sample the grass color from the texture using the sampler at this texture coordinate location.
	grassColour = grassTexture.Sample(texSampler, tex);

	// Sample the rock color from the texture using the sampler at this texture coordinate location.
	rockColour = rockTexture.Sample(texSampler, tex);

	// Sample the slope color from the texture using the sampler at this texture coordinate location.
	slopeColour = rockColour;
	slopeColour = saturate(slopeColour);

	// Calculate the slope of this point.
	slope = 1.0f - normal.y;

	// Determine which texture to use based on height.
	if (slope < 0.05)
	{
		blendAmount = slope / 0.05f;
		textureColour = lerp(grassColour, slopeColour, blendAmount);

		diffuseColour = doLighting(normal, worldPosition);
	}

	if ((slope < 0.6) && (slope >= 0.05f))
	{
		blendAmount = (slope - 0.05f) * (1.0f / (0.6f - 0.05f));
		textureColour = lerp(slopeColour, rockColour, blendAmount);

		diffuseColour = doLighting(normal, worldPosition);
	}

	if (slope >= 0.6)
	{
		textureColour = rockColour;

		diffuseColour = doLighting(normal, worldPosition);

	}

	return textureColour * diffuseColour;
}

float4 main(InputType input) : SV_TARGET
{
	float4 textureColour;

	//float3 lightVector = normalize(lightPosition - input.worldPosition);

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColour = slopeTexture(input.tex, input.normal, input.worldPosition);

	textureColour = saturate(textureColour);

	//if (showNormals)
	//{
	//	return float4(input.normal, 1.f);
	//}

	return textureColour;
}