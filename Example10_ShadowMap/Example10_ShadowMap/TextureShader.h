#include "DXF.h"
#include "MyLight.h"

using namespace std;
using namespace DirectX;

class TextureShader : public BaseShader
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
		float enabled;
	};

	struct LightBufferType
	{
		LightData Light[3];
	};

	struct NormalsBufferType
	{
		bool showNormals;
		XMFLOAT3 padding;
	};

public:
	TextureShader(ID3D11Device* device, HWND hwnd, bool isTerrain = false);
	~TextureShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture_one);
	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture_one, ID3D11ShaderResourceView* texture_two,  MyLight* light[], bool showNormals);

private:
	void initShader(WCHAR*, WCHAR*);

	// Buffers
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_normalBuffer;
};



