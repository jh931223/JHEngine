#include "stdafx.h"
#include "ResourcesClass.h"
#include "MeshClass.h"
#include "TextureClass.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "d3dclass.h"
#include "MaterialClass.h"

// 쉐이더들
#include "ShaderClass.h"

#include "RenderTextureClass.h"
#include "TriplanarShaderClass.h"
#include "DepthMapShaderClass.h"
#include "TextureShaderClass.h"

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

	result = new TriplanarShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["TriplanarShader"] = result;
	result = new DepthMapShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["DepthMapShader"] = result;
	result = new TextureShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["TextureShader"] = result;
	result = new TextureShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["TextureShaderTransparent"] = result;
}

void ResourcesClass::InitializeRenderTexture(HWND hwnd)
{
	RenderTextureClass* result;
	result = new RenderTextureClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), SystemClass::GetInstance()->GetWindowWidth()*5, SystemClass::GetInstance()->GetWindowHeight()*5, 2000, 10);
	rttMap["ShadowMap"] = result;
	result = new RenderTextureClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), SystemClass::GetInstance()->GetWindowWidth() * 4, SystemClass::GetInstance()->GetWindowHeight() * 4, 2000, 10);
	rttMap["BlurredShadowMap"] = result;
}

void ResourcesClass::InitializeMesh(HWND hwnd)
{
	meshMap["floor"] = new Mesh(SystemClass::GetInstance()->GetDevice(), "data/floor.txt");
	meshMap["cube"] = new Mesh(SystemClass::GetInstance()->GetDevice(), "data/cube.txt");
	meshMap["sphere"] = new Mesh(SystemClass::GetInstance()->GetDevice(), "data/sphere.txt");
	meshMap["windowOrtho"] = new Mesh(SystemClass::GetInstance()->GetDevice(), SystemClass::GetInstance()->GetWindowWidth(), SystemClass::GetInstance()->GetWindowHeight());
}

void ResourcesClass::InitializeTexture(HWND hwnd)
{
	textureMap["stone"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/stone.dds", false);
	textureMap["floor"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/seafloor.dds", false);
	textureMap["tile"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/tile.dds", false);
	textureMap["grass"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/grass.dds", false);
	textureMap["soil"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/soil.dds", false);
	textureMap["cliff"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Cliff_1.png");
	textureMap["cliffNormal"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Cliff_1_normal.png");
	textureMap["rock"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Rock_1.png");
	textureMap["rockNormal"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Rock_1_normal.png");
	textureMap["rock2"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Rock_2.png");
	textureMap["rock2Normal"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Rock_2_normal.png");
	textureMap["snow"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Snow_1.png");
	textureMap["snowNormal"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/SplatTexture/Snow_1_normal.png");
	textureMap["sky02"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/Sky_02.png");
	textureMap["sky01"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"data/Sky_01.png");
}

void ResourcesClass::InitializeMaterial(HWND hwnd)
{
	Material* result = new Material;

	result = new Material;
	result->SetShader(FindShader("TriplanarShader"));

	result->GetParams()->SetTexture("Texture1", FindTexture("cliff"));
	result->GetParams()->SetTexture("Normal1", FindTexture("cliffNormal"));
	result->GetParams()->SetTexture("Texture1_up", FindTexture("grass"));
	result->GetParams()->SetTexture("Normal1_up", FindTexture("cliffNormal"));

	result->GetParams()->SetTexture("Texture2", FindTexture("cliff"));
	result->GetParams()->SetTexture("Normal2", FindTexture("cliffNormal"));
	result->GetParams()->SetTexture("Texture2_up", FindTexture("snow"));
	result->GetParams()->SetTexture("Normal2_up", FindTexture("snowNormal"));

	result->GetParams()->SetTexture("Texture3", FindTexture("rock"));
	result->GetParams()->SetTexture("Normal3", FindTexture("rockNormal"));
	result->GetParams()->SetTexture("Texture3_up", FindTexture("grass"));
	result->GetParams()->SetTexture("Normal3_up", FindTexture("cliffNormal"));

	result->GetParams()->SetTexture("Texture4", FindTexture("rock2"));
	result->GetParams()->SetTexture("Normal4", FindTexture("rock2Normal"));
	result->GetParams()->SetTexture("Texture4_up", FindTexture("soil"));
	result->GetParams()->SetTexture("Normal4_up", FindTexture("cliffNormal"));

	result->GetParams()->SetFloat4("_MainColor", XMFLOAT4(1,1,1,1));
	materialMap["m_triplanar"] = result;

	result = new Material;
	result->SetShader(FindShader("TextureShader"));
	result->GetParams()->SetRenderTexture("_MainRenderTex", FindRenderTexture("ShadowMap"));
	materialMap["m_texture"] = result;

	result = new Material;
	result->SetShader(ResourcesClass::GetInstance()->FindShader("DepthMapShader"));
	materialMap["m_depthMap"] = result;

	result = new Material;
	result->SetShader(ResourcesClass::GetInstance()->FindShader("TextureShaderTransparent"));
	result->GetParams()->SetTexture("_MainTex", FindTexture("snow"));
	result->GetParams()->SetFloat4("_MainColor", XMFLOAT4(1,1,1,0.2f));
	result->Queue = QueueState::Transparent;
	materialMap["m_cube"] = result;


	result = new Material;
	result->SetShader(ResourcesClass::GetInstance()->FindShader("TextureShader"));
	result->GetParams()->SetTexture("_MainTex", FindTexture("sky02"));
	result->GetParams()->SetFloat4("_MainColor", XMFLOAT4(1, 1, 1, 1));
	materialMap["m_skySphere2"] = result;	
	result = new Material;
	result->SetShader(ResourcesClass::GetInstance()->FindShader("TextureShader"));
	result->GetParams()->SetTexture("_MainTex", FindTexture("sky01"));
	result->GetParams()->SetFloat4("_MainColor", XMFLOAT4(1, 1, 1, 1));
	materialMap["m_skySphere"] = result;

	result = new Material;
	result->SetShader(ResourcesClass::GetInstance()->FindShader("Blur_DepthMap"));
	result->GetParams()->SetRenderTexture("_ShadowMap", FindRenderTexture("ShadowMap"));
	materialMap["m_Blur_DepthMap"] = result;

}


void ResourcesClass::AddResource(std::string name, RenderTextureClass * _resource)
{
	rttMap[name] = _resource;
}

void ResourcesClass::AddResource(std::string name, TextureClass * _resource)
{
	textureMap[name] = _resource;
}

void ResourcesClass::AddResource(std::string name, Mesh * _resource)
{
	meshMap[name] = _resource;
}

void ResourcesClass::AddResource(std::string name, ShaderClass * _resource)
{
	shaderMap[name] = _resource;
}

void ResourcesClass::AddResource(std::string name, Material * _resource)
{
	materialMap[name] = _resource;
}
