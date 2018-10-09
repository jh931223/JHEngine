#include "stdafx.h"
#include "ResourcesClass.h"
#include "MeshClass.h"
#include "TextureClass.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "d3dclass.h"
#include "MaterialClass.h"

// 쉐이더들

#include "TextureShaderClass.h"
#include "ShadowShaderClass.h"
#include "RenderTextureClass.h"
#include "DepthShaderClass.h"
#include "MarchingCubeShaderClass.h"
#include "TriplanarShaderClass.h"

#include <map>
#include <string>

using namespace std;

ResourcesClass::ResourcesClass()
{
}


ResourcesClass::~ResourcesClass()
{	// 큐브 모델 오브젝트를 해제하십시오.
	for (auto &i:meshMap)
	{
		i.second->Shutdown();
		delete i.second;
		i.second = 0;
	}
	for (auto &i : materialMap)
	{
		delete i.second;
		i.second = 0;
	}
	for (auto &i : textureMap)
	{
		i.second->Shutdown();
		delete i.second;
		i.second = 0;
	}
	for (auto &i : shaderMap)
	{
		i.second->Shutdown();
		delete i.second;
		i.second = 0;
	}
}
void ResourcesClass::Initialize(HWND hwnd)
{
	InitializeMesh(hwnd);
	InitializeTexture(hwnd);
	InitializeShader(hwnd);
	InitializeRenderTexture(hwnd);
	InitializeMaterial(hwnd);
}


ResourcesClass * ResourcesClass::GetInstance()
{
	return SystemClass::GetInstance()->GetResources();
}

Material * ResourcesClass::FindMaterial(string _name)
{
	Material * m = materialMap[_name];
	if (!m)
		printf("Failed to find %s\n", _name.c_str());
	return m;
}

Mesh * ResourcesClass::FindMesh(string _name)
{
	return meshMap[_name];
}

TextureClass * ResourcesClass::FindTexture(string _name)
{
	return textureMap[_name];
}
ShaderClass * ResourcesClass::FindShader(string _name)
{
	ShaderClass * s = shaderMap[_name];
	if (s == NULL)
	{
		printf("Failed to find %s shader\n", _name.c_str());
	}
	return s;
}

RenderTextureClass * ResourcesClass::FindRenderTexture(std::string _name)
{
	return rttMap[_name];
}


void ResourcesClass::InitializeShader(HWND hwnd)
{
	ShaderClass* result;
	result = new TextureShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["TextureShader"] = result;

	result = new ShadowShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["ShadowShader"] = result;

	result = new DepthShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["DepthShader"] = result;

	result = new MarchingCubeShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["MarchingCubeShader"] = result;

	result = new TriplanarShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["TriplanarShader"] = result;
}

void ResourcesClass::InitializeRenderTexture(HWND hwnd)
{
	RenderTextureClass* result;
	result = new RenderTextureClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);
	rttMap["rt_Shadow"] = result;
}

void ResourcesClass::InitializeMesh(HWND hwnd)
{
	//meshMap["floor"] = new Mesh(SystemClass::GetInstance()->GetDevice(), "../JHEngine/data/floor.txt");
	//meshMap["cube"] = new Mesh(SystemClass::GetInstance()->GetDevice(), "../JHEngine/data/cube.txt");
}

void ResourcesClass::InitializeTexture(HWND hwnd)
{
	textureMap["stone"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/stone.dds", false);
	textureMap["floor"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/seafloor.dds", false);
	textureMap["tile"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/tile.dds", false);
	textureMap["grass"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/grass.dds", false);
	textureMap["soil"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/soil.dds",false);
	textureMap["cliff"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Cliff_1.png");
	textureMap["cliffNormal"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Cliff_1_normal.png");
	textureMap["rock"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Rock_1.png");
	textureMap["rockNormal"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Rock_1_normal.png");
	textureMap["rock2"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Rock_2.png");
	textureMap["rock2Normal"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Rock_2_normal.png");
	textureMap["snow"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Snow_1.png");
	textureMap["snowNormal"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Snow_1_normal.png");
}

void ResourcesClass::InitializeMaterial(HWND hwnd)
{
	Material* result = new Material;
	result->SetShader(new ShadowShaderClass,hwnd);
	result->GetParams()->SetTexture("Texture", FindTexture("stone"));
	result->GetParams()->SetRenderTexture("DepthMapTexture", FindRenderTexture("rt_Shadow"));
	result->GetParams()->SetFloat4("ambientColor", XMFLOAT4(1.0, 1.0, 1, 1));
	result->GetParams()->SetFloat4("diffuseColor", XMFLOAT4(4, 1.0, 0, 1));
	materialMap["cube"] = result;

	result = new Material;
	result->SetShader(new ShadowShaderClass, hwnd);
	result->GetParams()->SetTexture("Texture", FindTexture("floor"));
	result->GetParams()->SetRenderTexture("DepthMapTexture", FindRenderTexture("rt_Shadow"));
	result->GetParams()->SetFloat4("ambientColor", XMFLOAT4(1,1.0,1,1));
	result->GetParams()->SetFloat4("diffuseColor", XMFLOAT4(1, 1.0, 0, 1));
	materialMap["floor"] = result;

	result = new Material;
	result->SetShader(new DepthShaderClass, hwnd);
	materialMap["depthMap"] = result;

	result = new Material;
	result->SetShader(FindShader("MarchingCubeShader"), hwnd);
	result->GetParams()->SetTexture("Texture1", FindTexture("soil"));
	result->GetParams()->SetTexture("Texture2", FindTexture("grass"));
	result->GetParams()->SetTexture("Texture3", FindTexture("soil"));
	materialMap["m_marchingCube"] = result;	
	
	result = new Material;
	result->SetShader(new TextureShaderClass, hwnd);
	result->GetParams()->SetTexture("Texture", FindTexture("tile"));
	materialMap["m_texture"] = result;

	result = new Material;
	result->SetShader(FindShader("TriplanarShader"), hwnd);
	result->GetParams()->SetTexture("Texture1", FindTexture("cliff"));
	result->GetParams()->SetTexture("Texture1Normal", FindTexture("cliffNormal"));
	result->GetParams()->SetTexture("Texture2", FindTexture("grass"));
	result->GetParams()->SetTexture("Texture2Normal", FindTexture("rockNormal"));
	result->GetParams()->SetTexture("Texture3", FindTexture("cliff"));
	result->GetParams()->SetTexture("Texture3Normal", FindTexture("cliffNormal"));
	materialMap["m_triplanar"] = result;
}


