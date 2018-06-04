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
	return materialMap[_name];
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
	return shaderMap[_name];
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
	textureMap["stone"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"../JHEngine/data/stone.dds");
	textureMap["floor"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"../JHEngine/data/seafloor.dds");
	textureMap["tile"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"../JHEngine/data/tile.dds");
	textureMap["grass"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"../JHEngine/data/grass.dds");
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
	result->SetShader(new MarchingCubeShaderClass, hwnd);
	result->GetParams()->SetTexture("Texture", FindTexture("tile"));
	materialMap["m_marchingCube"] = result;	result = new Material;

	result->SetShader(new TextureShaderClass, hwnd);
	result->GetParams()->SetTexture("Texture", FindTexture("tile"));
	materialMap["m_texture"] = result;
}


