#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	m_grassClipPlane = XMFLOAT4(0.0f, 1.0f, 0.0f, 3);

	m_doCelShading = true;
	m_doTessellation = false;
	m_doOutline = true;
	m_normalMode = false;

	//createBlendState();

	//renderer->getDeviceContext()->OMSetBlendState(m_blend, blendFactor, sampleMask);

	createCullingStates();

	// Initialise tessellation vars
	m_maxTessDist = 1;
	m_minTessDist = 0;
	m_maxTessFactor = 1;

	// Initialise grass vars
	m_grassDensity = 1;
	m_grassWidth = 1.f;
	m_grassHeight = 1.f;

	// Initialise utline variables
	m_edgeWidth = 0.5f;
	m_edgeIntensity = 5.0f;
	m_normalThreshold = 1.f;
	m_depthThreshold = 0.0f;
	m_normalSensitivity = 1.f;
	m_depthSensitivity = 1.f;

	// Initialise screen size 
	m_screen.y = BaseApplication::sHeight;
	m_screen.x = BaseApplication::sWidth;

	// Initialise scene objects and ortho meshes
	m_map = new Terrain(renderer->getDevice(), renderer->getDeviceContext());
	m_celShaderMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size
	m_outlineShaderMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size
	m_finalMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size
	m_skyBox = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/Sphere.obj");
	m_castle = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/cabin.obj");
	m_windmill = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/windmill.obj");
	m_water = new Water(renderer->getDevice(), renderer->getDeviceContext(), 80);

	// Load textures
	textureMgr->loadTexture("dock1", L"../res/dockwood.png");
	textureMgr->loadTexture("grassBillboard", L"../res/desertGrass.png");
	textureMgr->loadTexture("skybox", L"../res/skyobx.png");
	textureMgr->loadTexture("displacement", L"../res/rockbump.png");
	textureMgr->loadTexture("wood", L"../res/wood.png");
	textureMgr->loadTexture("heightmap", L"../res/height.png");
	textureMgr->loadTexture("grass", L"../res/sand.png");
	textureMgr->loadTexture("dirt", L"../res/rock.png");
	textureMgr->loadTexture("rockNorm", L"../res/rocknormal.png");
	textureMgr->loadTexture("grassNorm", L"../res/sandnormal.png");
	textureMgr->loadTexture("waterNorm", L"../res/waternormal.dds");
	textureMgr->loadTexture("castle", L"../res/cabin.png");
	textureMgr->loadTexture("windmill", L"../res/Windmill.png");

	// Initialise shaders
	m_textureShader = new TextureShader(renderer->getDevice(), hwnd);
	m_depthShader = new DepthShader(renderer->getDevice(), hwnd);
	m_celShader = new CelShader(renderer->getDevice(), hwnd);
	m_outlineShader = new OutlineShader(renderer->getDevice(), hwnd);
	m_tessShader = new TessellationShader(renderer->getDevice(), hwnd);
	m_grasShader = new TessellationShader(renderer->getDevice(), hwnd, true);
	m_tessCelShader = new CelShader(renderer->getDevice(), hwnd, true);
	m_reflectionShader = new ReflectionShader(renderer->getDevice(), hwnd);
	m_waterShader = new WaterShader(renderer->getDevice(), hwnd);
	m_terrainCelShader = new CelShader(renderer->getDevice(), hwnd, false, true);
	m_terrainTextureShader = new TextureShader(renderer->getDevice(), hwnd, true);

	// Init shadow map vars
	int shadowmapWidth = 2048;
	int shadowmapHeight = 2048;

	// Initialise render textures
	m_shadowMap = new RenderTexture(renderer->getDevice(), shadowmapWidth, shadowmapHeight, 0.1f, 100.f);
	m_outlineShaderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	m_celShaderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	m_renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	m_reflectionRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	m_refractionRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Modifyables for light one
	m_lightOneCol[0] = 1.0f;	m_lightOnePos[0] = 10.0f;
	m_lightOneCol[1] = 1.0f;	m_lightOnePos[1] = 10.0f;
	m_lightOneCol[2] = 1.0f;	m_lightOnePos[2] = 50.0f;

	//Modifyables for light two
	m_lightTwoCol[0] = 1.0f;	m_lightTwoPos[0] = 25.0f;
	m_lightTwoCol[1] = 1.0f;	m_lightTwoPos[1] = 10.0f;
	m_lightTwoCol[2] = 1.0f;	m_lightTwoPos[2] = 50.0f;

	//Modifyables for light three
	m_lightThreeCol[0] = 0.2f;	m_lightThreeDirection[0] = 0.0f;
	m_lightThreeCol[1] = 0.2f;	m_lightThreeDirection[1] = -1.0f;
	m_lightThreeCol[2] = 0.2f;	m_lightThreeDirection[2] = 0.0f;

	// Initialise lights
	for (int i = 0; i < 3; i++) {
		m_light[i] = new MyLight(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT3(50.f, 10.f, 10.f), XMFLOAT3(0.0f, -1.0f, 0.0f), 1.0f, 0.125f, 0.0f, DIRECTIONAL, 0.f);
	}
	// Set cam pos
	camera->setPosition(-10.f, 10.f, 100.f);
	camera->setRotation(0.f, 180.f, 0.f);

	m_lightThreeDirection[0] = 0.0f;
	m_lightThreeDirection[1] = 1.0f;
	m_lightThreeDirection[2] = 1.0f;
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	for (int i = 0; i < 3; i++) {

		if (m_light[i])
		{
			delete m_light[i];
			m_light[i] = 0;
		}
	}

	if (m_map)
	{
		delete m_map;
		m_map = 0;
	};

	if (m_skyBox)
	{
		delete m_skyBox;
		m_skyBox = 0;
	};

	if (m_castle)
	{
		delete m_castle;
		m_castle = 0;
	};

	if (m_windmill)
	{
		delete m_windmill;
		m_windmill = 0;
	};

	if (m_water)
	{
		delete m_water;
		m_water = 0;
	};

	if (m_celShaderMesh)
	{
		delete m_celShaderMesh;
		m_celShaderMesh = 0;
	};

	if (m_outlineShaderMesh)
	{
		delete m_outlineShaderMesh;
		m_outlineShaderMesh = 0;
	};

	if (m_finalMesh)
	{
		delete m_finalMesh;
		m_finalMesh = 0;
	};

	// Release shaders
	if (m_depthShader)
	{
		delete m_depthShader;
		m_depthShader = 0;
	};

	if (m_outlineShader)
	{
		delete m_outlineShader;
		m_outlineShader = 0;
	};

	if (m_textureShader)
	{
		delete m_textureShader;
		m_textureShader = 0;
	};

	if (m_terrainTextureShader)
	{
		delete m_terrainTextureShader;
		m_terrainTextureShader = 0;
	};
	// Release shaders
	if (m_celShader)
	{
		delete m_celShader;
		m_celShader = 0;
	};

	if (m_terrainCelShader)
	{
		delete m_terrainCelShader;
		m_terrainCelShader = 0;
	};

	if (m_tessCelShader)
	{
		delete m_tessCelShader;
		m_tessCelShader = 0;
	};

	if (m_tessShader)
	{
		delete m_tessShader;
		m_tessShader = 0;
	};

	if (m_grasShader)
	{
		delete m_grasShader;
		m_grasShader = 0;
	};
	// Release shaders
	if (m_reflectionShader)
	{
		delete m_reflectionShader;
		m_reflectionShader = 0;
	};

	if (m_waterShader)
	{
		delete m_waterShader;
		m_waterShader = 0;
	};

	////
	if (m_shadowMap)
	{
		delete m_shadowMap;
		m_shadowMap = 0;
	};

	if (m_renderTexture)
	{
		delete m_renderTexture;
		m_renderTexture = 0;
	};

	if (m_outlineShaderTexture)
	{
		delete m_outlineShaderTexture;
		m_outlineShaderTexture = 0;
	};

	if (m_celShaderTexture)
	{
		delete m_celShaderTexture;
		m_celShaderTexture = 0;
	};

	if (m_reflectionRenderTexture)
	{
		delete m_reflectionRenderTexture;
		m_reflectionRenderTexture = 0;
	};

	if (m_refractionRenderTexture)
	{
		delete m_refractionRenderTexture;
		m_refractionRenderTexture = 0;
	};

	if (m_rasterStateNoCulling)
	{
		delete m_rasterStateNoCulling;
		m_rasterStateNoCulling = 0;
	};

	if (m_rasterStateCulling)
	{
		delete m_rasterStateCulling;
		m_rasterStateCulling = 0;
	};

	if (m_blend)
	{
		delete m_blend;
		m_blend = 0;
	};
}

bool App1::frame()
{
	bool result;

	// Get scren size
	m_screen.y = BaseApplication::sHeight;
	m_screen.x = BaseApplication::sWidth;

	// Translate light to cam position
	//m_light[0]->setPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);

	// Increment variable for animation of water
	m_waterTranslation += 0.0005f;

	if (m_waterTranslation > 0.45f)
	{
		m_waterTranslation -= 0.45f;
	}

	result = BaseApplication::frame();

	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	m_normalMode = false;

	// Render refract to texture
	renderRefraction();

	// render reflect to texture
	renderReflection();

	if (!m_doCelShading)
	{
		// Render scene normally
		renderPass();
	}
	else
	{
		// Fill in 'cel'
		celPass();
	}

	// Get outline of scene
	outlinePass();
	// Render scene
	finalPass();

	return true;
}

void App1::renderRefraction()
{
	// Set the render target to be the render to texture and clear it
	m_refractionRenderTexture->setRenderTarget(renderer->getDeviceContext());
	m_refractionRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.6f, 0.6f, 0.6f, 1.0f);

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	m_waterClipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, 2.f + 0.1f);

	// Move terrain 
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	// Render terrain as normal
	m_map->sendData(renderer->getDeviceContext());
	m_reflectionShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("grass"), textureMgr->getTexture("grassNorm"), m_waterClipPlane, m_light);
	m_reflectionShader->render(renderer->getDeviceContext(), m_map->getIndexCount());

	renderer->setBackBufferRenderTarget();
}

void App1::renderReflection()
{
	// Set the render target to be the render to texture and clear it
	m_reflectionRenderTexture->setRenderTarget(renderer->getDeviceContext());
	m_reflectionRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.6f, 0.6f, 0.6f, 1.0f);

	XMFLOAT3 cameraPosition;

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = getReflectionMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	m_waterClipPlane = XMFLOAT4(0.0f, 1.0f, 0.0f, -2);
	// Move terrain 
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	// Render terrain as normal
	m_map->sendData(renderer->getDeviceContext());
	m_reflectionShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("grass"), textureMgr->getTexture("grassNorm"), m_waterClipPlane, m_light);
	m_reflectionShader->render(renderer->getDeviceContext(), m_map->getIndexCount());

	// Render skybox
	// Get the position of the camera.
	cameraPosition = camera->getPosition();

	// Invert the Y coordinate of the camera around the water plane height for the reflected camera position.
	cameraPosition.y = -cameraPosition.y + (2.f * 2.0f);

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixScaling(10.f, 10.f, 10.f);
	worldMatrix *= XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);


	// Turn off back face culling and disable Z buffer.
	renderer->setZBuffer(false);
	turnOffCulling();

		m_skyBox->sendData(renderer->getDeviceContext());
		m_textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("skybox"));
		m_textureShader->render(renderer->getDeviceContext(), m_skyBox->getIndexCount());

	turnOnCulling();
	renderer->setZBuffer(true);

	renderer->setBackBufferRenderTarget();

}

void App1::renderPass()
{
	// Set the render target to be the render to texture and clear it
	m_renderTexture->setRenderTarget(renderer->getDeviceContext());
	m_renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.6f, 0.6f, 0.6f, 1.0f);

	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX reflectionMatrix = getReflectionMatrix();

	// Move grass 
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	// Turn off culling for grass
	turnOffCulling();

		// Render grass from terrain mesh
		m_map->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		m_grasShader->setGrassShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("grassBillboard"), camera->getPosition(), m_light, m_grassClipPlane, m_normalMode);
		m_grasShader->render(renderer->getDeviceContext(), m_map->getIndexCount());

	turnOnCulling();

	// Render skybox
	worldMatrix = XMMatrixScaling(10.f, 10.f, 10.f);
	worldMatrix *= XMMatrixTranslation(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);

	m_skyBox->sendData(renderer->getDeviceContext());
	m_textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("skybox"));
	m_textureShader->render(renderer->getDeviceContext(), m_skyBox->getIndexCount());

	// Move terrain 
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	if (m_doTessellation)
	{
		m_map->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		m_tessShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("grass"), textureMgr->getTexture("dirt"), textureMgr->getTexture("grassNorm"), textureMgr->getTexture("rockNorm"), textureMgr->getTexture("displacement"), camera->getPosition(), m_light, m_normalMode);
		m_tessShader->render(renderer->getDeviceContext(), m_map->getIndexCount());
	}
	else
	{
		// Render terrain as normal
		m_map->sendData(renderer->getDeviceContext());
		m_terrainTextureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("grass"), textureMgr->getTexture("dirt"), m_light, m_normalMode);
		m_terrainTextureShader->render(renderer->getDeviceContext(), m_map->getIndexCount());
	}

	// Render skybox
	worldMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	worldMatrix *= XMMatrixRotationY(3.25f);
	worldMatrix *= XMMatrixTranslation(-5.f, 6.f, 35.f);

	//m_castle->sendData(renderer->getDeviceContext());
	//m_celShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("castle"), m_shadowMap->getShaderResourceView(), m_light);
	//m_celShader->render(renderer->getDeviceContext(), m_castle->getIndexCount());

	// Translate to the location of the water and render it.
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(-45.0f, 2.f, 10.f);

	m_water->sendData(renderer->getDeviceContext());
	m_waterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix, m_reflectionRenderTexture->getShaderResourceView(), m_refractionRenderTexture->getShaderResourceView(), textureMgr->getTexture("waterNorm"), camera->getPosition(), m_water->getNormRepeat(), m_waterTranslation, m_water->getReflectRefractScale(), m_water->getRefractTint(), m_light[0]->getDirection(), 90.f);
	m_waterShader->render(renderer->getDeviceContext(), m_water->getIndexCount());

	renderer->setBackBufferRenderTarget();
}

void App1::celPass()
{
	// Set the render target to be the render to texture and clear it
	m_renderTexture->setRenderTarget(renderer->getDeviceContext());
	m_renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.6f, 0.6f, 0.6f, 1.0f);
	
	camera->update();

	// Composite matrice transform
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();	// Default camera position for orthographic rendering
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX reflectionMatrix = getReflectionMatrix();

	// Move grass 
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	turnOffCulling();
		
		// Render grass from terrain mesh
		m_map->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		m_grasShader->setGrassShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("grassBillboard"), camera->getPosition(), m_light, m_grassClipPlane, m_normalMode);
		m_grasShader->render(renderer->getDeviceContext(), m_map->getIndexCount());

	turnOnCulling();

	// Render skybox
	worldMatrix = XMMatrixScaling(10.f, 10.f, 10.f);
	worldMatrix *= XMMatrixTranslation(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);

	m_skyBox->sendData(renderer->getDeviceContext());
	m_textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("skybox"));
	m_textureShader->render(renderer->getDeviceContext(), m_skyBox->getIndexCount());

	// Move terrain 
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	

	if (m_doTessellation)
	{
		// Render terrain as normal
		m_map->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		m_tessCelShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("grass"), textureMgr->getTexture("dirt"), textureMgr->getTexture("grassNorm"), textureMgr->getTexture("rockNorm"), textureMgr->getTexture("displacement"), camera->getPosition(), m_light, m_normalMode);
		m_tessCelShader->render(renderer->getDeviceContext(), m_map->getIndexCount());
	}
	else
	{
		// Render terrain as normal
		m_map->sendData(renderer->getDeviceContext());
		m_terrainCelShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("grass"), textureMgr->getTexture("dirt"), m_shadowMap->getShaderResourceView(), m_light, m_normalMode);
		m_terrainCelShader->render(renderer->getDeviceContext(), m_map->getIndexCount());
	}

	// Render skybox
	worldMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	worldMatrix *= XMMatrixRotationY(3.25f);
	worldMatrix *= XMMatrixTranslation(-5.f, 6.f, 35.f);

	//m_castle->sendData(renderer->getDeviceContext());
	//m_celShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("castle"), m_shadowMap->getShaderResourceView(), m_light);
	//m_celShader->render(renderer->getDeviceContext(), m_castle->getIndexCount());

	// Translate to the location of the water and render it.
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(-45.0f, 2.f, 10.f);

	m_water->sendData(renderer->getDeviceContext());
	m_waterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix, m_reflectionRenderTexture->getShaderResourceView(), m_refractionRenderTexture->getShaderResourceView(), textureMgr->getTexture("waterNorm"), camera->getPosition(), m_water->getNormRepeat(), m_waterTranslation, m_water->getReflectRefractScale(), m_water->getRefractTint(), m_light[0]->getDirection(), 90.f);
	m_waterShader->render(renderer->getDeviceContext(), m_water->getIndexCount());

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::outlinePass()
{
	// RENDER THE RENDER TEXTURE SCENE
	// Requires 2D rendering and an ortho mesh.
	renderer->setZBuffer(false);

	// Set the render target to be the render to texture and clear it
	m_outlineShaderTexture->setRenderTarget(renderer->getDeviceContext());
	m_outlineShaderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.6f, 0.6f, 0.6f, 1.0f);

	// Get matrices
	camera->update();

	// Composite matrice transform
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering

	m_outlineShaderMesh->sendData(renderer->getDeviceContext());
	m_outlineShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, m_renderTexture->getShaderResourceView(), m_screen);
	m_outlineShader->render(renderer->getDeviceContext(), m_outlineShaderMesh->getIndexCount());

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.6f, 0.6f, 0.6f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	m_finalMesh->sendData(renderer->getDeviceContext());

	if (m_doOutline)
	{
		m_textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, m_outlineShaderTexture->getShaderResourceView());
	}
	else
	{
		m_textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, m_renderTexture->getShaderResourceView());
	}

	m_textureShader->render(renderer->getDeviceContext(), m_finalMesh->getIndexCount());

	renderer->setZBuffer(true);

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);
	
	// Build UI
	ImFont* font = ImGui::GetFont();

	ImGui::PushFont(font);

	ImGui::Begin("Testooo");

	// Constants
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text("Trans: %.2f", m_waterTranslation);
	ImGui::Checkbox("Normal mode", &m_normalMode);

	// Cel shading / outline
	if (ImGui::CollapsingHeader("Cel shading/Outline options"))
	{
		ImGui::Checkbox("Toggle Cel Shading", &m_doCelShading);
		ImGui::Checkbox("Toggle Outline", &m_doOutline);
		ImGui::SliderFloat("Edge Width", &m_edgeWidth, 0.0f, 20.f, "%.1f");
		ImGui::SliderFloat("Edge Intensity", &m_edgeIntensity, 0.0f, 5.f, "%.1f");
		ImGui::SliderFloat("Normal Threshold", &m_normalThreshold, 0.0f, 1.f, "%.1f");
		ImGui::SliderFloat("Depth Threshold", &m_depthThreshold, 0.0f, 1.f, "%.1f");
		ImGui::SliderFloat("Normal Sensitivity", &m_normalSensitivity, 0.0f, 1.f,"%.1f");
		ImGui::SliderFloat("Depth Sensitivity", &m_depthSensitivity, 0.0f, 1.f, "%.1f");
		ImGui::DragFloat3("Pos", pos, 1.f, -100.f, 100.f);
		}

	// Tessellation
	if (ImGui::CollapsingHeader("Tessellation options"))
	{
		ImGui::Checkbox("Toggle Tessellation/Displacement", &m_doTessellation);
		ImGui::SliderInt("Max Tess Distance", &m_maxTessDist, 0.0f, 5.f);
		ImGui::SliderInt("Min Tess Distance", &m_minTessDist, 0.0f, 5.f);
		ImGui::SliderInt("Tess Factor", &m_maxTessFactor, 0.0f, 20.f);
	}
	// Grass
	if (ImGui::CollapsingHeader("Grass options"))
	{
		ImGui::SliderInt("Grass Density", &m_grassDensity, 0.0f, 5.f);
		ImGui::SliderFloat("Grass Height", &m_grassHeight, 0.0f, 5.f);
		ImGui::SliderFloat("Grass Width", &m_grassWidth, 0.0f, 5.f);
	}
	// Lights
	if (ImGui::CollapsingHeader("Light options"))
	{	
		ImGui::Text("Light Type 0 = DIRECTIONAL");
		ImGui::Text("Light Type 1 = POINTLIGHT");
		ImGui::Text("Light Type 2 = SPOTLIGHT");

		if (ImGui::CollapsingHeader("Light One options"))
		{
			ImGui::ColorEdit3("Light One Colour", (float*)&m_lightOneCol);
			ImGui::DragFloat3("Light One Position", (float*)&m_lightOnePos);
			ImGui::SliderInt("Light One Type", &m_lightOneType, 0, 2);
			ImGui::Checkbox("Light One On/Off", &m_lightOneEnabled);
		}
		if (ImGui::CollapsingHeader("Light Two options"))
		{
			ImGui::ColorEdit3("Light Two Colour", (float*)&m_lightTwoCol);
			ImGui::DragFloat3("Light Two Position", (float*)&m_lightTwoPos);
			ImGui::SliderInt("Light Two Type", &m_lightTwoType, 0, 2);
			ImGui::Checkbox("Light Two On/Off", &m_lightTwoEnabled);
		}
		if (ImGui::CollapsingHeader("Light Three options"))
		{
			ImGui::Text("Light Three is a directional light, its type and position cannot be changed");
			ImGui::ColorEdit3("Light Three Colour", (float*)&m_lightThreeCol);
			ImGui::DragFloat3("Light Three Direction", (float*)&m_lightThreeDirection);
			ImGui::Checkbox("Light Three On/Off", &m_lightThreeEnabled);
		}
	}

	ImGui::PopFont();
	ImGui::End();

	// Update tessellation variables in respective shader class
	m_tessShader->setMaxTessDist(m_maxTessDist);		m_tessCelShader->setMaxTessDist(m_maxTessDist);
	m_tessShader->setMinTessDist(m_minTessDist);		m_tessCelShader->setMinTessDist(m_minTessDist);
	m_tessShader->setMaxTessFactor(m_maxTessFactor);	m_tessCelShader->setMaxTessFactor(m_maxTessFactor);
	m_tessShader->setMinTessFactor(m_minTessFactor);	m_tessCelShader->setMinTessFactor(m_minTessFactor);

	// Update grass variables
	m_grasShader->setGrassDensity(m_grassDensity);		
	m_grasShader->setGrassSize(m_grassWidth, m_grassHeight);

	// Update outline variables
	m_outlineShader->setOutlineWidth(m_edgeWidth);
	m_outlineShader->setOutlineIntensity(m_edgeIntensity);
	m_outlineShader->setThresholds(m_normalThreshold, m_depthThreshold);
	m_outlineShader->setSensitivity(m_normalSensitivity, m_depthSensitivity);

	// Update light variables
	// Light One
	m_light[0]->setDiffuseColour(m_lightOneCol[0], m_lightOneCol[1], m_lightOneCol[2], 1.0f);
	m_light[0]->setPosition(m_lightOnePos[0], m_lightOnePos[1], m_lightOnePos[2]);
	m_light[0]->setLightType((LightType)m_lightOneType);
	
	if (m_doCelShading)
	{
		m_light[0]->setEnabled(1.f);
		m_lightOneEnabled = true;
	}
	else
	{
		if(m_lightOneEnabled)
		m_light[0]->setEnabled(1.f);
		else
		m_light[0]->setEnabled(0.f);
	}

	// Light Two
	m_light[1]->setDiffuseColour(m_lightTwoCol[0], m_lightTwoCol[1], m_lightTwoCol[2], 1.0f);
	m_light[1]->setPosition(m_lightTwoPos[0], m_lightTwoPos[1], m_lightTwoPos[2]);
	m_light[1]->setLightType((LightType)m_lightTwoType);

	if (m_doCelShading)
	{
		m_light[1]->setEnabled(0.0f);
		m_lightTwoEnabled = false;
	}
	else
	{
		if (m_lightTwoEnabled)
			m_light[1]->setEnabled(1.f);
		else
			m_light[1]->setEnabled(0.f);
	}

	// Light Three
	m_light[2]->setDiffuseColour(m_lightThreeCol[0], m_lightThreeCol[1], m_lightThreeCol[2], 1.0f);
	m_light[2]->setDirection(m_lightThreeDirection[0], m_lightThreeDirection[1], m_lightThreeDirection[2]);

	if (m_lightThreeEnabled)
		m_light[2]->setEnabled(1.f);
	else
		m_light[2]->setEnabled(0.f);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

XMMATRIX App1::getReflectionMatrix()
{
	XMFLOAT3 up, position, lookAt;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;
	XMMATRIX reflectionMatrix;

	XMVECTOR vecRot = camera->getRotation();

	XMFLOAT3 floatRot = XMFLOAT3(XMVectorGetX(vecRot), XMVectorGetY(vecRot), XMVectorGetZ(vecRot));

	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Setup the position of the camera in the world.  For planar reflection invert the Y position of the camera.
	position.x = camera->getPosition().x;
	position.y = -camera->getPosition().y + (2 * 2);
	position.z = camera->getPosition().z;

	// Setup where the camera is looking by default.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.  Invert the X rotation for reflection.
	pitch = -floatRot.x * 0.0174532925f;
	yaw = floatRot.y * 0.0174532925f;
	roll = floatRot.z * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	XMVECTOR lookatVec = XMVectorSet(lookAt.x, lookAt.y, lookAt.z, 1.0f);
	
	lookatVec = XMVector3TransformCoord(lookatVec, rotationMatrix);

	XMVECTOR upVec = XMVectorSet(up.x, up.y, up.z, 1.0f);

	upVec = XMVector3TransformCoord(upVec, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	XMVECTOR positionVec = XMVectorSet(position.x, position.y, position.z, 1.0f);

	lookatVec = positionVec + lookatVec;

	// Finally create the reflection view matrix from the three updated vectors.
	reflectionMatrix = XMMatrixLookAtLH(positionVec, lookatVec, upVec);

	return reflectionMatrix;
}

void App1::createBlendState()
{
	D3D11_BLEND_DESC blendStateDescription;

	// Setup a raster description which turns off back face culling.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the no culling rasterizer state.
	renderer->getDevice()->CreateBlendState(&blendStateDescription, &m_blend);
}

void App1::createCullingStates()
{
	D3D11_RASTERIZER_DESC rasterStateDesc;

	// Disable back face culling
	rasterStateDesc.AntialiasedLineEnable = false;
	rasterStateDesc.CullMode = D3D11_CULL_NONE;
	rasterStateDesc.DepthBias = 0;
	rasterStateDesc.DepthBiasClamp = 0.0f;
	rasterStateDesc.DepthClipEnable = true;
	rasterStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterStateDesc.FrontCounterClockwise = false;
	rasterStateDesc.MultisampleEnable = false;
	rasterStateDesc.ScissorEnable = false;
	rasterStateDesc.SlopeScaledDepthBias = 0.0f;

	// Create state
	renderer->getDevice()->CreateRasterizerState(&rasterStateDesc, &m_rasterStateNoCulling);

	// Enable back face culling.
	rasterStateDesc.AntialiasedLineEnable = false;
	rasterStateDesc.CullMode = D3D11_CULL_FRONT;
	rasterStateDesc.DepthBias = 0;
	rasterStateDesc.DepthBiasClamp = 0.0f;
	rasterStateDesc.DepthClipEnable = true;
	rasterStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterStateDesc.FrontCounterClockwise = false;
	rasterStateDesc.MultisampleEnable = false;
	rasterStateDesc.ScissorEnable = false;
	rasterStateDesc.SlopeScaledDepthBias = 0.0f;

	// Create state
	renderer->getDevice()->CreateRasterizerState(&rasterStateDesc, &m_rasterStateCulling);
}

void App1::turnOffCulling()
{
	renderer->getDeviceContext()->RSSetState(m_rasterStateNoCulling);
}

void App1::turnOnCulling()
{
	renderer->getDeviceContext()->RSSetState(m_rasterStateCulling);
}

