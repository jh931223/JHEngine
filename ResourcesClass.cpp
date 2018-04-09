#include "stdafx.h"
#include "ResourcesClass.h"
#include "MeshClass.h"
#include "TextureClass.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "d3dclass.h"
#include "MaterialClass.h"
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
	InitializeMaterial(hwnd);
}


ResourcesClass * ResourcesClass::GetInstance()
{
	return SystemClass::GetInstance()->GetResources();
}

MaterialClass * ResourcesClass::FindMaterial(string _name)
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


void ResourcesClass::InitializeShader(HWND hwnd)
{
	ShaderClass* result;
	result = new TextureShaderClass;
	result->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
	shaderMap["TextureShader"] = result;
}

void ResourcesClass::InitializeMesh(HWND hwnd)
{
	meshMap["floor"] = new Mesh(SystemClass::GetInstance()->GetDevice(), "../JHEngine/data/floor.txt");
	meshMap["cube"] = new Mesh(SystemClass::GetInstance()->GetDevice(), "../JHEngine/data/cube.txt");
}

void ResourcesClass::InitializeTexture(HWND hwnd)
{
	textureMap["stone"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"../JHEngine/data/stone.dds");
	textureMap["floor"] = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"../JHEngine/data/seafloor.dds");
}

void ResourcesClass::InitializeMaterial(HWND hwnd)
{
	MaterialClass* result = new MaterialClass;
	result->SetShader(FindShader("TextureShader"));
	result->SetParameter<TextureClass*>("Texture",FindTexture("stone"));
	materialMap["cube"] = result;

	result = new MaterialClass;
	result->SetShader(FindShader("TextureShader"));
	result->SetParameter<TextureClass*>("Texture", FindTexture("floor"));
	materialMap["floor"] = result;
}


