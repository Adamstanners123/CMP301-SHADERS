#include "include\BaseShader.h"

using namespace std;
using namespace DirectX;

class OutlineShader : public BaseShader
{
private:
	struct ScreenBufferType
	{
		float screenWidth;
		float screenHeight;
		XMFLOAT2 padding;
	};

	struct OutlineDataBufferType
	{
		float edgeWidth;
		float edgeIntensity;
		float normalThreshold;
		float depthThreshold;
		float normalSensitivity;
		float depthSensitivity;
		XMFLOAT2 padding;
	};

public:
	OutlineShader(ID3D11Device* device, HWND hwnd);
	~OutlineShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, XMFLOAT2 screen);

	void setOutlineWidth(float width);
	void setOutlineIntensity(float intensity);
	void setThresholds(float normal, float depth);
	void setSensitivity(float normal, float depth);

private:
	void initShader(WCHAR*, WCHAR*);

private:

	// Buffers
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11SamplerState* m_sampler;
	ID3D11Buffer* m_screenBuffer;
	ID3D11Buffer* m_outlineBuffer;

	// Outline vars
	float m_edgeWidth;
	float m_edgeIntensity;
	float m_normalThreshold;
	float m_depthThreshold;
	float m_normalSensitivity;
	float m_depthSensitivity;
};


