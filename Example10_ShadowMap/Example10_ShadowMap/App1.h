// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "OutlineShader.h"
#include "CelShader.h"
#include "Terrain.h"
#include "TessellationShader.h"
#include "Water.h"
#include "ReflectionShader.h"
#include "WaterShader.h"
#include "MyLight.h"
#include "LightType.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void renderRefraction();
	void renderReflection();
	void renderPass();
	void celPass();
	void outlinePass();
	void finalPass();
	void gui();


private:

	XMMATRIX getReflectionMatrix();

	void createBlendState();
	void createCullingStates();
	void turnOffCulling();
	void turnOnCulling();

	// Lights
	MyLight* m_light[3];

	// Scene objects
	Terrain* m_map;
	Model* m_skyBox;
	Model* m_castle;
	Model* m_windmill;
	Water* m_water;

	// Orthos for 2D rendering
	OrthoMesh* m_celShaderMesh;
	OrthoMesh* m_outlineShaderMesh;
	OrthoMesh* m_finalMesh;

	// Shaders
	DepthShader* m_depthShader;
	OutlineShader* m_outlineShader;
	TextureShader* m_textureShader;
	TextureShader* m_terrainTextureShader;
	CelShader* m_celShader;
	CelShader* m_terrainCelShader;
	CelShader* m_tessCelShader;
	TessellationShader* m_tessShader; 
	TessellationShader* m_grasShader;
	ReflectionShader* m_reflectionShader;
	WaterShader* m_waterShader;

	// Render textures
	RenderTexture* m_shadowMap;
	RenderTexture* m_renderTexture;
	RenderTexture* m_outlineShaderTexture;
	RenderTexture* m_celShaderTexture;
	RenderTexture* m_reflectionRenderTexture;
	RenderTexture* m_refractionRenderTexture;

	// Culling vars
	ID3D11RasterizerState* m_rasterStateNoCulling = 0;
	ID3D11RasterizerState* m_rasterStateCulling = 0;

	// Blend vars
	ID3D11BlendState* m_blend;

	// GUI toggles
	bool m_doCelShading;
	bool m_doTessellation;
	bool m_doOutline;
	bool m_normalMode;

	// Tessellation variables
	int m_maxTessDist;
	int m_minTessDist;
	int m_maxTessFactor;
	int m_minTessFactor;

	// Outline variables
	float m_edgeWidth;
	float m_edgeIntensity;
	float m_normalThreshold;
	float m_depthThreshold;
	float m_normalSensitivity;
	float m_depthSensitivity;

	// float position
	float pos[3];

	// Grass variables
	int m_grassDensity;
	float m_grassWidth;
	float m_grassHeight;

	// Screen width/height
	XMFLOAT2 m_screen;

	// For water animation
	float m_waterTranslation;

	// Clipping planes
	XMFLOAT4 m_waterClipPlane;
	XMFLOAT4 m_grassClipPlane;

	// Light vars
	float m_lightOneCol[3], m_lightTwoCol[3], m_lightThreeCol[3];
	float  m_lightOnePos[3], m_lightTwoPos[3];
	float m_lightThreeDirection[3];
	bool m_lightOneEnabled = true, m_lightTwoEnabled = false, m_lightThreeEnabled = false;
	int m_lightOneType = 1, m_lightTwoType = 2;
};

#endif