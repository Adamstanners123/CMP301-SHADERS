#include "WaterShader.h"

WaterShader::WaterShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"water_vs.cso", L"water_ps.cso");
}

WaterShader::~WaterShader()
{
	// Release the sampler state.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the pixel shader water data constant buffer.
	if (m_psWaterDataBuffer)
	{
		m_psWaterDataBuffer->Release();
		m_psWaterDataBuffer = 0;
	}

	// Release the vertex shader water data constant buffer.
	if (m_vsWaterDataBuffer)
	{
		m_vsWaterDataBuffer->Release();
		m_vsWaterDataBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void WaterShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC psWaterDataDesc;
	D3D11_BUFFER_DESC vsWaterDataDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
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
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &m_sampleState);

	// Setup the description of the pixel shader water data dynamic constant buffer that is in the vertex shader.
	psWaterDataDesc.Usage = D3D11_USAGE_DYNAMIC;
	psWaterDataDesc.ByteWidth = sizeof(PSWaterDataType);
	psWaterDataDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psWaterDataDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psWaterDataDesc.MiscFlags = 0;
	psWaterDataDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&psWaterDataDesc, NULL, &m_psWaterDataBuffer);

	// Setup the description of the vertex shader water data dynamic constant buffer that is in the vertex shader.
	vsWaterDataDesc.Usage = D3D11_USAGE_DYNAMIC;
	vsWaterDataDesc.ByteWidth = sizeof(VSWaterDataType);
	vsWaterDataDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsWaterDataDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsWaterDataDesc.MiscFlags = 0;
	vsWaterDataDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&vsWaterDataDesc, NULL, &m_vsWaterDataBuffer);
}

void WaterShader::setShaderParameters(ID3D11DeviceContext * deviceContext, const XMMATRIX & worldMatrix, const XMMATRIX & viewMatrix, const XMMATRIX & projectionMatrix, 
									  const XMMATRIX & reflectionMatrix, ID3D11ShaderResourceView * reflectTexture, ID3D11ShaderResourceView * refractTexture, 
									  ID3D11ShaderResourceView * normalTexture, XMFLOAT3 camPos, XMFLOAT2 normRepeat, float translation, float reflectRefractScale, 
									  XMFLOAT4 refractTint, XMFLOAT3 lightDirection, float specShininess)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	PSWaterDataType* psWaterPtr;
	VSWaterDataType* vsWaterPtr;

	XMMATRIX tworld, tview, tproj, trefl;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	trefl = XMMatrixTranspose(reflectionMatrix);

	// Sned matrix data
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dataPtr->reflection = trefl;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	
	// Sned vertex water data
	result = deviceContext->Map(m_vsWaterDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	vsWaterPtr = (VSWaterDataType*)mappedResource.pData;
	vsWaterPtr->camPos = camPos;// worldMatrix;
	vsWaterPtr->normRepeat = normRepeat;
	vsWaterPtr->padding = XMFLOAT3(0.f, 0.f, 0.f);
	deviceContext->Unmap(m_vsWaterDataBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &m_vsWaterDataBuffer);

	// Send pixel water data to pixel shader
	deviceContext->Map(m_psWaterDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	psWaterPtr = (PSWaterDataType*)mappedResource.pData;
	psWaterPtr->refractTint = refractTint;
	psWaterPtr->lightDirection = lightDirection;
	psWaterPtr->translation = translation;
	psWaterPtr->reflectRefractScale = reflectRefractScale;
	psWaterPtr->specShinniness = specShininess;
	psWaterPtr->padding = XMFLOAT2(0.f, 0.f);
	deviceContext->Unmap(m_psWaterDataBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_psWaterDataBuffer);

	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &refractTexture);
	deviceContext->PSSetShaderResources(1, 1, &reflectTexture);
	deviceContext->PSSetShaderResources(2, 1, &normalTexture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}