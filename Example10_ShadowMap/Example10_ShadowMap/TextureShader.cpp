#include "TextureShader.h"


TextureShader::TextureShader(ID3D11Device* device, HWND hwnd, bool isTerrain) : BaseShader(device, hwnd)
{
	if (isTerrain)
	{
		initShader(L"texture_vs.cso", L"terraintexture_ps.cso");
	}
	else
	{
		initShader(L"texture_vs.cso", L"texture_ps.cso");
	}

}

TextureShader::~TextureShader()
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

	// Release the light data constant buffer.
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	// Release the normal buffer
	if (m_normalBuffer)
	{
		m_normalBuffer->Release();
		m_normalBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void TextureShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC mormalBufferDesc;

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
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &m_sampleState);

	// Setup light buffer
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Setup the normal buffer.
	mormalBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	mormalBufferDesc.ByteWidth = sizeof(NormalsBufferType);
	mormalBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mormalBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mormalBufferDesc.MiscFlags = 0;
	mormalBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&mormalBufferDesc, NULL, &m_normalBuffer);

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);

}

// Normal shader setup
void TextureShader::setShaderParameters(ID3D11DeviceContext * deviceContext, const XMMATRIX & worldMatrix, const XMMATRIX & viewMatrix, const XMMATRIX & projectionMatrix, ID3D11ShaderResourceView * texture_one)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Sned matrix data
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture_one);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

}

// Multi-texturing shader setup
void TextureShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture_one, ID3D11ShaderResourceView* texture_two, MyLight* light[], bool showNormals)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Sned matrix data
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	for (int i = 0; i < 3; i++) {

		lightPtr->Light[i].ambient = light[i]->getAmbientColour();
		lightPtr->Light[i].diffuse = light[i]->getDiffuseColour();
		lightPtr->Light[i].position = light[i]->getPosition();
		lightPtr->Light[i].constantAttenuation = light[i]->getConstantAttenuation();
		lightPtr->Light[i].linearAttenuation = light[i]->getLinearAttenuation();
		lightPtr->Light[i].quadraticAttenuation = light[i]->getQuadraticAttenuation();
		lightPtr->Light[i].lightType = light[i]->getLightType();
		lightPtr->Light[i].enabled = light[i]->getEnabled();
		lightPtr->Light[i].spotAngle = 45.0;
		lightPtr->Light[i].direction = light[i]->getDirection();

	}
	deviceContext->Unmap(m_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_lightBuffer);

	// For displaying normals
	NormalsBufferType* nrmPtr;
	deviceContext->Map(m_normalBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	nrmPtr = (NormalsBufferType*)mappedResource.pData;
	nrmPtr->showNormals = showNormals;
	nrmPtr->padding = XMFLOAT3(0.f, 0.f, 0.f);
	deviceContext->Unmap(m_normalBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &m_normalBuffer);

	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture_one);
	deviceContext->PSSetShaderResources(1, 1, &texture_two);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
}




