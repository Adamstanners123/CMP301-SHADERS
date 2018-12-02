#pragma once

#include "DXF.h"
#include "MyLight.h"

using namespace std;
using namespace DirectX;

class CelShader : public BaseShader
{
private:
	
	struct TessellationBufferType
	{
		int maxTessellationDist;
		int minTessellationDist;
		int tessellationFactor;
		XMFLOAT3 cameraPosition;
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
	CelShader(ID3D11Device* device, HWND hwnd, bool doTessellation = false, bool isTerrain = false);
	~CelShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture_one, ID3D11ShaderResourceView* shadowMap, MyLight* light[]);
	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture_one, ID3D11ShaderResourceView* texture_two, ID3D11ShaderResourceView* shadowMap, MyLight* light[], bool showNormals);
	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture_one, ID3D11ShaderResourceView* texture_two, ID3D11ShaderResourceView* grassNormMap, ID3D11ShaderResourceView* rockNormMap, ID3D11ShaderResourceView* dispMap, XMFLOAT3 cameraPosition, MyLight* light[], bool showNormals);

	void setMaxTessDist(int x);
	void setMinTessDist(int x);
	void setMaxTessFactor(int x);
	void setMinTessFactor(int x);

private:
	void initShader(WCHAR*, WCHAR*);
	void initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename);
	void loadDispMapVertexShader(WCHAR* filename);

private:
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_tessellationBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_normalBuffer;

	int m_maxTessFactor;
	int m_minTessFactor;
	int m_maxTessDist;
	int m_minTessDist;

};

