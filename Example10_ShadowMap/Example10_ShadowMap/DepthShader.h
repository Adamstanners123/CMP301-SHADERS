#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{
public:

	DepthShader(ID3D11Device* device, HWND hwnd);
	~DepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	// Buffers
	ID3D11SamplerState * m_sampleState;
	ID3D11Buffer* m_matrixBuffer;
};
