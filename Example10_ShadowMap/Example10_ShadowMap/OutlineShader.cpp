#include "OutlineShader.h"


OutlineShader::OutlineShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	// Init shaders
	initShader(L"outline_vs.cso", L"outline_ps.cso");
}

OutlineShader::~OutlineShader()
{
	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release sampler.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release sampler.
	if (m_sampler)
	{
		m_sampler->Release();
		m_sampler = 0;
	}

	// Release the screen size constant buffer.
	if (m_screenBuffer)
	{
		m_screenBuffer->Release();
		m_screenBuffer = 0;
	}

	// Release the outline data constant buffer.
	if (m_screenBuffer)
	{
		m_screenBuffer->Release();
		m_screenBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void OutlineShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC screenBufferDesc;
	D3D11_BUFFER_DESC outlineBufferDesc;

	// Load (+ compile) shader files
	loadTextureVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &m_sampleState);

	renderer->CreateSamplerState(&samplerDesc, &m_sampler);

	// Setup screen size buffer
	screenBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	screenBufferDesc.ByteWidth = sizeof(ScreenBufferType);
	screenBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	screenBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	screenBufferDesc.MiscFlags = 0;
	screenBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&screenBufferDesc, NULL, &m_screenBuffer);

	// Setup outline data buffer
	outlineBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	outlineBufferDesc.ByteWidth = sizeof(ScreenBufferType);
	outlineBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	outlineBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	outlineBufferDesc.MiscFlags = 0;
	outlineBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&outlineBufferDesc, NULL, &m_outlineBuffer);

}

void OutlineShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT2 screen)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Sned matrix data
	deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Send screen size data to PS
	ScreenBufferType* screenPtr;
	deviceContext->Map(m_screenBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	screenPtr = (ScreenBufferType*)mappedResource.pData;
	screenPtr->screenHeight = screen.y;
	screenPtr->screenWidth = screen.x;
	screenPtr->padding = XMFLOAT2(0.0f, 0.0f);
	deviceContext->Unmap(m_screenBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_screenBuffer);

	// Send screen size data to PS
	OutlineDataBufferType* outlinePtr;
	deviceContext->Map(m_outlineBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	outlinePtr = (OutlineDataBufferType*)mappedResource.pData;
	outlinePtr->edgeWidth = m_edgeWidth;
	outlinePtr->edgeIntensity = m_edgeIntensity;
	outlinePtr->normalThreshold = m_normalThreshold;
	outlinePtr->depthThreshold = m_depthThreshold;
	outlinePtr->normalSensitivity = m_normalSensitivity;
	outlinePtr->depthSensitivity = m_depthSensitivity;
	outlinePtr->padding = XMFLOAT2(0.0f, 0.0f);
	deviceContext->Unmap(m_outlineBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &m_outlineBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->PSSetSamplers(1, 1, &m_sampler);
}

void OutlineShader::setOutlineWidth(float width)
{
	m_edgeWidth = width;
}

void OutlineShader::setOutlineIntensity(float intensity)
{
	m_edgeIntensity = intensity;
}

void OutlineShader::setThresholds(float normal, float depth)
{
	m_normalThreshold = normal;
	m_depthThreshold = depth;
}

void OutlineShader::setSensitivity(float normal, float depth)
{
	m_normalSensitivity = normal;
	m_depthSensitivity = depth;
}

