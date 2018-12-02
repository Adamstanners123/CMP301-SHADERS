// Max number of lights
#define MAX_LIGHTS 3

// Light Types
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

Texture2D grassTexture : register(t0);
Texture2D rockTexture : register(t1);

Texture2D rockNormalMap : register(t2);
Texture2D grassNormalMap : register(t3);

SamplerState celSampler : register(s0);

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
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 camPos : TEXCOORD1;
	float3 worldPosition : TEXCOORD2;
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

float4 doCelShading(float3 lightVector, float3 normal)
{
	float4 celColour;

	// Multiply the texture pixel and the final diffuse color to get the final pixel color result.
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
	float ndl = max(0, dot(normal, lightVector));

	if (ndl > ToonThresholds[0])
	{
		celColour = ToonBrightnessLevels[0];
	}
	else if (ndl > ToonThresholds[1])
	{
		celColour = ToonBrightnessLevels[1];
	}
	else if (ndl > ToonThresholds[2])
	{
		celColour = ToonBrightnessLevels[2];
	}
	else if (ndl > ToonThresholds[3])
	{
		celColour = ToonBrightnessLevels[3];
	}
	else
	{
		celColour = ToonBrightnessLevels[4];
	}

	//celColour = saturate(celColour);

	return celColour;
}

float4 doLighting(float3 bumpNormal, float3 worldPosition, float4 textureColour)
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
			finalColour += calculateDirectionalLight(Lights[i], bumpNormal);				// Calculate directional light
			break;
		}
		case POINT_LIGHT:
		{
			lightVector = normalize(Lights[i].position - worldPosition);				// Calculate point light
			finalColour += calculatePointLight(Lights[i], lightVector, bumpNormal);
			textureColour *= doCelShading(lightVector, bumpNormal);
			break;
		}
		case SPOT_LIGHT:
		{
			lightVector = normalize(Lights[i].position - worldPosition);				// Calculate spot light
			finalColour += calculateSpotLight(Lights[i], lightVector, bumpNormal);
			textureColour *= doCelShading(lightVector, bumpNormal);
			break;
		}

		}

	}

	finalColour = textureColour * finalColour;

	finalColour = saturate(finalColour);

	return finalColour;

}

float3 calculateBumpNormal(float2 tex, float3 tangent, float3 binormal, float3 normal, bool rockOrSand)
{
	float3 bumpMap;
	float3 bumpNormal;

	// Calculate the amount of light on this pixel using the normal map.
	if (rockOrSand)
	{
		bumpMap = grassNormalMap.Sample(celSampler, tex).rgb;
	}
	else
	{
		bumpMap = rockNormalMap.Sample(celSampler, tex).rgb;
	};

	bumpMap = (bumpMap * 2.0f) - 1.0f;

	// Build orthonormal basis.
	float3 N = normal;
	float3 T = normalize(tangent - dot(tangent, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	bumpNormal = mul(bumpMap, TBN);

	return bumpNormal;
}

float4 slopeTexture(float2 tex, float3 normal, float3 tangent, float3 binormal, float3 worldPosition)
{
	float4 grassColour;
	float4 slopeColour;
	float4 rockColour;
	float4 textureColour;
	float4 diffuseColour;
	float3 norm;
	float slope;
	float blendAmount;
	float3 lightVector;

	// Sample the grass color from the texture using the sampler at this texture coordinate location.
	grassColour = grassTexture.Sample(celSampler, tex);

	// Sample the rock color from the texture using the sampler at this texture coordinate location.
	rockColour = rockTexture.Sample(celSampler, tex);

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

		norm = calculateBumpNormal(tex, tangent, binormal, normal, true);
		diffuseColour = doLighting(norm, worldPosition, textureColour);
	}

	if ((slope < 0.6) && (slope >= 0.05f))
	{
		blendAmount = (slope - 0.05f) * (1.0f / (0.6f - 0.05f));
		textureColour = lerp(slopeColour, rockColour, blendAmount);

		norm = calculateBumpNormal(tex, tangent, binormal, normal, true);
		diffuseColour = doLighting(norm, worldPosition, textureColour);
	}

	if (slope >= 0.6)
	{
		textureColour = rockColour;

		norm = calculateBumpNormal(tex, tangent, binormal, normal, false);
		diffuseColour = doLighting(norm, worldPosition, textureColour);
	}

	return diffuseColour;
}

float4 main(InputType input) : SV_TARGET
{
	float4 colour;

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
		finalColour += calculateDirectionalLight(Lights[i], input.normal);				// Calculate directional light
		break;
	}
	case POINT_LIGHT:
	{
		lightVector = normalize(Lights[i].position - input.worldPosition);				// Calculate point light
		finalColour += calculatePointLight(Lights[i], lightVector, input.normal);
		break;
	}
	case SPOT_LIGHT:
	{
		lightVector = normalize(Lights[i].position - input.worldPosition);				// Calculate spot light
		finalColour += calculateSpotLight(Lights[i], lightVector, input.normal);
		break;
	}

	}

}
	// Applies slope based texture and does celShading/Lighting
	colour = slopeTexture(input.tex, input.normal, input.tangent, input.binormal, input.worldPosition);

	return colour;
}