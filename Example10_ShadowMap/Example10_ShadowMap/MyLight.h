#pragma once

#include "DXF.h"
#include "LightType.h"

using namespace std;
using namespace DirectX;

class MyLight : public Light
{
public:
	MyLight(XMFLOAT4 diffuse, XMFLOAT4 ambient, XMFLOAT3 position, XMFLOAT3 direction, float constant, float linear, float quadratic, LightType type, float enabled);
	~MyLight();
	
	//Setters
	void setConstantAttenuation(float x);
	void setLinearAttenuation(float x);
	void setQuadraticAttenuation(float x);
	void setLightType(LightType type);
	void setEnabled(float x);
	
	//Getters
	float getConstantAttenuation();
	float getLinearAttenuation();
	float getQuadraticAttenuation();
	LightType getLightType();
	float getEnabled();

protected:

	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	LightType lightType;
	float enabled;
};

