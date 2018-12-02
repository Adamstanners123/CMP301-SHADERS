#include "Water.h"

Water::Water(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int size) : PlaneMesh(device, deviceContext, size)
{
	// Set the tiling for the water normal maps
	m_normalRepeat.x = size / 100.f;  
	m_normalRepeat.y = size / 50.f;  

    // Set the scaling value for the water effects
	m_reflectRefractScale = 0.3f;

	// Set the tint of the refraction
	m_refractTint = XMFLOAT4(0.1f, 0.3f, 1.0f, 1.0f);

	initBuffers(device);
}

Water::~Water()
{
	// Run parent deconstructor
	PlaneMesh::~PlaneMesh();
}

XMFLOAT2 Water::getNormRepeat()
{
	return m_normalRepeat;
}

XMFLOAT4 Water::getRefractTint()
{
	return m_refractTint;
}

float Water::getReflectRefractScale()
{
	return m_reflectRefractScale;
}


