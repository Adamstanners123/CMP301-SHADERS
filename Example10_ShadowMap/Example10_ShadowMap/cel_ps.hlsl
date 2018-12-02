// Max number of lights
#define MAX_LIGHTS 3

// Light Types
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

Texture2D objTexture : register(t0);
Texture2D depthMapTexture : register(t1);

SamplerState celSampler  : register(s0);

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
	bool enabled;
};

cbuffer LightBuffer : register(b0)
{
	Light Lights[MAX_LIGHTS];
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
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
		if (!Lights[i].enabled) continue;

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
	float3 lightVector = normalize(Lights[2].position - input.worldPosition);

	float4 celColor = objTexture.Sample(celSampler, input.tex);

	float ToonThresholds[4];
	ToonThresholds[0] = 0.95;
	ToonThresholds[1] = 0.5;
	ToonThresholds[2] = 0.2;
	ToonThresholds[3] = 0.03;

	float ToonBrightnessLevels[5];
	ToonBrightnessLevels[0] = 1.0;
	ToonBrightnessLevels[1] = 0.8;
	ToonBrightnessLevels[2] = 0.6;
	ToonBrightnessLevels[3] = 0.4;
	ToonBrightnessLevels[4] = 0.2;

	// diffuse
	float ndl = max(0, dot(input.normal, lightVector));

	if (ndl > ToonThresholds[0])
	{
		celColor *= ToonBrightnessLevels[0];
	}
	else if (ndl > ToonThresholds[1])
	{
		celColor *= ToonBrightnessLevels[1];
	}
	else if (ndl > ToonThresholds[2])
	{
		celColor *= ToonBrightnessLevels[2];
	}
	else if (ndl > ToonThresholds[3])
	{
		celColor *= ToonBrightnessLevels[3];
	}
	else
	{
		celColor *= ToonBrightnessLevels[4];
	}

	//return float4(input.normal, 1.f);

	celColor *= doLighting(input.normal, input.worldPosition);

	return celColor;
}
