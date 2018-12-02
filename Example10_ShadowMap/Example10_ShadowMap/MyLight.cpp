#include "MyLight.h"



MyLight::MyLight(XMFLOAT4 diffuse, XMFLOAT4 ambient, XMFLOAT3 position, XMFLOAT3 direction, float constant, float linear, float quadratic, LightType type, float enabled)
{
	setAmbientColour(ambient.x, ambient.y, ambient.z, ambient.w);
	setDiffuseColour(diffuse.x, diffuse.y, diffuse.z, diffuse.w);
	setPosition(position.x, position.y, position.z);
	setDirection(direction.x, direction.y, direction.z);
	setConstantAttenuation(constant);
	setLinearAttenuation(linear);
	setQuadraticAttenuation(quadratic);
	setEnabled(enabled);
	setLightType(type);
}


MyLight::~MyLight()
{
}

void MyLight::setConstantAttenuation(float x)
{
	constantAttenuation = x;
}

void MyLight::setLinearAttenuation(float x)
{
	linearAttenuation = x;
}

void MyLight::setQuadraticAttenuation(float x)
{
	quadraticAttenuation = x;
}

void MyLight::setLightType(LightType type)
{
	lightType = type;
}

void MyLight::setEnabled(float x)
{
	enabled = x;
}



float MyLight::getConstantAttenuation()
{
	return constantAttenuation;
}

float MyLight::getLinearAttenuation()
{
	return linearAttenuation;
}

float MyLight::getQuadraticAttenuation()
{
	return quadraticAttenuation;
}

LightType MyLight::getLightType()
{
	return lightType;
}

float MyLight::getEnabled()
{
	return enabled;
}
