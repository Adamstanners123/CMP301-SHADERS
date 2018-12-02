#pragma once
#include "include\BaseShader.h"

#include "DXF.h"
#include "MyLight.h"

using namespace std;
using namespace DirectX;

class ReflectionShader : public BaseShader
{
private:

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
		bool enabled;
	};

	struct LightBufferType
	{
		LightData Light[3];
	};

	// Clip plane data
	struct ClipPlaneBufferType
	{
		XMFLOAT4 clipPlane;
	};

public:
	ReflectionShader(ID3D11Device* device, HWND hwnd);
	~ReflectionShader();

	void setShaderParameters(ID3D11DeviceContext * deviceContext, const XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection,
							 ID3D11ShaderResourceView * texture, ID3D11ShaderResourceView * normal_texture, XMFLOAT4 clipPlane, MyLight* light[]);

private:
	void initShader(WCHAR*, WCHAR*);
	void loadReflectVertexShader(WCHAR*);

	// Buffers
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_clipPlaneBuffer;
	ID3D11Buffer* m_lightBuffer;
};

