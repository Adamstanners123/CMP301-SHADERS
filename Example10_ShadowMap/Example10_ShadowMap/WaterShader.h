#pragma once
#include "include\BaseShader.h"

#include "DXF.h"

using namespace std;
using namespace DirectX;

class WaterShader : public BaseShader
{
private:

	// Matrix data
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX reflection;
	};

	// Pixel shader water data
	struct PSWaterDataType
	{
		XMFLOAT4 refractTint;
		XMFLOAT3 lightDirection;
		float translation;
		float reflectRefractScale;
		float specShinniness;
		XMFLOAT2 padding;
	};

	// Vertex shader water data
	struct VSWaterDataType
	{
		XMFLOAT3 camPos;
		XMFLOAT2 normRepeat;
		XMFLOAT3 padding;
	};

public:
	WaterShader(ID3D11Device* device, HWND hwnd);
	~WaterShader();
	
	void setShaderParameters(ID3D11DeviceContext * deviceContext, const XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, const XMMATRIX & reflection,
							 ID3D11ShaderResourceView * reflectTexture, ID3D11ShaderResourceView * refractTexture, ID3D11ShaderResourceView * normalTexture, 
							 XMFLOAT3 camPos, XMFLOAT2 normRepeat, float translation, float reflectRefractScale, XMFLOAT4 refractTint, XMFLOAT3 lightDirection,
							 float specShininess);

private:
	void initShader(WCHAR*, WCHAR*);

	// Buffers
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_psWaterDataBuffer;
	ID3D11Buffer* m_vsWaterDataBuffer;
};

