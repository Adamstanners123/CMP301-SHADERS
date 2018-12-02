#pragma once
#include "include\DXF.h"
#include <vector>
#include <iostream>     // std::cout
#include <sstream>

using namespace DirectX;

class Terrain : public BaseMesh
{
public:
	struct HeightMapData {       
		int terrainWidth;        
		int terrainHeight;      
		XMFLOAT3 *heightMap;   
	};

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
	};

	Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~Terrain();

	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	bool heightMapLoad(char* filename, HeightMapData &hminfo);
	void createVertices();
	void calculateNormals();
	void calculateNormalsSum(XMFLOAT3* tempNorms);
	void calculateIndices();
	void fillNewTerrainVerts();
	void calculateTerrainVector();
	void calculateBiNormal(VertexType temp_verts_one, VertexType temp_verts_two, VertexType temp_verts_three, XMFLOAT3& tangent);

protected:
	void initBuffers(ID3D11Device* device);

	HRESULT m_result;

	// Containers for heightmap verts, normals etc..
	HeightMapData m_bmpHeightMap;
	VertexType* m_terrainVerts;
	VertexType* m_newTerrainVerts;

	XMFLOAT3* tempNorms;

	// Create the index array.
	unsigned long* m_indices;
};

