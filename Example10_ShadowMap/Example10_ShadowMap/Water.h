#pragma once
#include "include\DXF.h"

// BaseMesh class with getters to avoid cluttering app.cpp with rendering vars
class Water : public PlaneMesh
{
public:

	Water(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int size);
	~Water();

	//virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	XMFLOAT2 getNormRepeat();
	XMFLOAT4 getRefractTint();
	float	 getReflectRefractScale();

protected:
	//void initBuffers(ID3D11Device* device);

	// Water rendering vars
	float m_reflectRefractScale;
	XMFLOAT2 m_normalRepeat;
	XMFLOAT4 m_refractTint;

	//VertexType* m_vertices;
	//unsigned long* m_indices;
};

