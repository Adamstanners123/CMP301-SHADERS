// Light shader.h
// Basic single light shader setup
#pragma once

#include "DXF.h"
#include "MyLight.h"
#include <d3dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")

using namespace std;
using namespace DirectX;


class TessellationShader : public BaseShader
{
private:
	
	// Tessellation data
	struct TessellationBufferType
	{
		int maxTessellationDist;
		int minTessellationDist;
		int tessellationFactor;
		XMFLOAT3 cameraPosition;
		XMFLOAT2 padding;
	};

	// Tessellation data
	struct GrassBufferType
	{
		int grassTessFactor;
		XMFLOAT3 cameraPosition;
		XMFLOAT4 grassClipPlane;
	};

	struct GrassSizeBufferTyoe
	{
		float grassHeight;
		float grassWidth;
		XMFLOAT2 padding;
	};

	// light data
	struct LightData
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 position;
		float spotAngle;
		XMFLOAT3 direction;
		float constantAttenuation;
		float linearAttenuation;
		float quadraticAttenuation;
		int lightType;
		float enabled;
	};

	struct LightBufferType
	{
		LightData Light[3];
	};

public:

	TessellationShader(ID3D11Device* device, HWND hwnd, bool doGrass = false);
	~TessellationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
							 ID3D11ShaderResourceView* texture_one, ID3D11ShaderResourceView* texture_two, ID3D11ShaderResourceView* grassNormMap, 
						     ID3D11ShaderResourceView* rockNormMap, ID3D11ShaderResourceView* dispMap, XMFLOAT3 cameraPosition, MyLight* light[], bool showNormals);

	void setGrassShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
								  ID3D11ShaderResourceView* texture_one, XMFLOAT3 cameraPosition, MyLight* light[], XMFLOAT4 grassClipPlane, bool showNormals);

	// Setters for changing variables that control tessellation
	void setMaxTessDist(int x);
	void setMinTessDist(int x);
	void setMaxTessFactor(int x);
	void setMinTessFactor(int x);
	void setGrassDensity(int x);
	void setGrassSize(float x, float y);

private:
	void initShader(WCHAR* vsFilename, WCHAR* psFilename);
	void initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename);
	void initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* gsFilename, WCHAR* psFilename);
	void loadDispMapVertexShader(WCHAR* filename);

	// Buffers
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_tessellationBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_grassBuffer;
	ID3D11Buffer* m_grassSizeBuffer;
	ID3D11Buffer* m_normalBuffer;

	// Tessellation variables
	int m_maxTessFactor;
	int m_minTessFactor;
	int m_maxTessDist;
	int m_minTessDist;

	// Grass variables
	int m_grassTessFactor;
	float m_grassHeight;
	float m_grassWidth;
	bool m_doGrass;
};
