#include "CelShader.h"

CelShader::CelShader(ID3D11Device* device, HWND hwnd, bool doTessellation, bool isTerrain) : BaseShader(device, hwnd)
{
	// Init shaders
	if (doTessellation)
	{
		initShader(L"terrainnormdisp_vs.cso", L"terrainnormdisp_hs.cso", L"terrainnormdisp_ds.cso", L"terrainceldisp_ps.cso");
	}
	else if (!doTessellation && !isTerrain)
	{
		initShader(L"cel_vs.cso", L"cel_ps.cso");
	}
	else
	{
		initShader(L"cel_vs.cso", L"terraincel_ps.cso");
	}
}

CelShader::~CelShader()
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

	// Release the light constant buffer.
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	// Release the tessellation constant buffer.
	if (m_tessellationBuffer)
	{
		m_tessellationBuffer->Release();
		m_tessellationBuffer = 0;
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

void CelShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC tessellationBufferDesc;

	// Load (+ compile) shader files
	loadDispMapVertexShader(vsFilename);
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

	// Setup tessellation buffer for VS
	tessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessellationBufferDesc.ByteWidth = sizeof(TessellationBufferType);
	tessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessellationBufferDesc.MiscFlags = 0;
	tessellationBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&tessellationBufferDesc, NULL, &m_tessellationBuffer);

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
	renderer->CreateSamplerState(&samplerDesc, &m_sampleState);

	// Setup light buffer for PS
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
}

void CelShader::initShader(WCHAR * vsFilename, WCHAR * hsFilename, WCHAR * dsFilename, WCHAR * psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}

void CelShader::loadDispMapVertexShader(WCHAR * filename)
{
	ID3DBlob* vertexShaderBuffer;

	unsigned int numElements;

	vertexShaderBuffer = 0;

	// check file extension for correct loading function.
	std::wstring fn(filename);
	std::string::size_type idx;
	std::wstring extension;

	idx = fn.rfind('.');

	if (idx != std::string::npos)
	{
		extension = fn.substr(idx + 1);
	}

	else
	{
		// No extension found
		MessageBox(hwnd, L"Error finding vertex shader file", L"ERROR", MB_OK);
		exit(0);
	}

	// Load the texture in.
	if (extension != L"cso")
	{
		MessageBox(hwnd, L"Incorrect vertex shader file type", L"ERROR", MB_OK);
		exit(0);
	}

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(filename, &vertexShaderBuffer);

	if (result != S_OK)
	{
		MessageBox(NULL, filename, L"File ERROR", MB_OK);
		exit(0);
	}

	// Create the vertex shader from the buffer.
	renderer->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader);

	// Create the vertex input layout description.
	// Custom layorut adds support for tangents and binormals.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {

	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	renderer->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &layout);

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;
}


void CelShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture_one, ID3D11ShaderResourceView* shadowMap, MyLight *light[])
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Sned matrix data to VS
	deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	//Additional
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

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture_one);
	deviceContext->PSSetShaderResources(1, 1, &shadowMap);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

}

void CelShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture_one, ID3D11ShaderResourceView* texture_two, ID3D11ShaderResourceView* shadowMap, MyLight* light[], bool showNormals)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Sned matrix data to VS
	deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	//Additional
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

	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture_one);
	deviceContext->PSSetShaderResources(1, 1, &texture_two);
	deviceContext->PSSetShaderResources(2, 1, &shadowMap);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
}

void CelShader::setShaderParameters(ID3D11DeviceContext * deviceContext, const XMMATRIX & worldMatrix, const XMMATRIX & viewMatrix, 
									const XMMATRIX & projectionMatrix, ID3D11ShaderResourceView * texture_one, ID3D11ShaderResourceView * texture_two,
									ID3D11ShaderResourceView * grassNormMap, ID3D11ShaderResourceView * rockNormMap, ID3D11ShaderResourceView * dispMap,
									XMFLOAT3 cameraPosition, MyLight * light[], bool showNormals)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	LightBufferType* lightPtr;
	TessellationBufferType* tessPtr;
	MatrixBufferType* dataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Sned matrix data to VS
	deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Sned matrix data to DS
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Send tessellation vars to VS
	result = deviceContext->Map(m_tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tessPtr = (TessellationBufferType*)mappedResource.pData;
	tessPtr->maxTessellationDist = m_maxTessDist;// worldMatrix;
	tessPtr->minTessellationDist = m_minTessDist;
	tessPtr->tessellationFactor = m_maxTessFactor;
	tessPtr->cameraPosition = cameraPosition;
	tessPtr->padding = XMFLOAT2(0.0f, 0.0f);
	deviceContext->Unmap(m_tessellationBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &m_tessellationBuffer);

	// Sned light data
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

	// Set shader texture resource in the domain shader.
	deviceContext->DSSetShaderResources(0, 1, &dispMap);
	deviceContext->DSSetSamplers(0, 1, &m_sampleState);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture_one);
	deviceContext->PSSetShaderResources(1, 1, &texture_two);
	deviceContext->PSSetShaderResources(2, 1, &rockNormMap);
	deviceContext->PSSetShaderResources(3, 1, &grassNormMap);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
}

void CelShader::setMaxTessDist(int x)
{
	m_maxTessDist = x;
}

void CelShader::setMinTessDist(int x)
{
	m_minTessDist = x;
}

void CelShader::setMaxTessFactor(int x)
{
	m_maxTessFactor = x;
}

void CelShader::setMinTessFactor(int x)
{
	m_minTessFactor = x;
}
