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
{	// ť�� �� ������Ʈ�� �����Ͻʽÿ�.
	if (m_CubeModel)
	{
		m_CubeModel->Shutdown();
		delete m_CubeModel;
		m_CubeModel = 0;
	}
	// �׶��� �� ��ü�� �����Ѵ�.
	if (m_GroundModel)
	{
		m_GroundModel->Shutdown();
		delete m_GroundModel;
		m_GroundModel = 0;
	}
	if (t_cube)
	{
		t_cube->Shutdown();
		delete t_cube;
		t_cube = 0;
	}
	if (t_Floor)
	{
		t_Floor->Shutdown();
		delete t_Floor;
		t_Floor = 0;
	}
	if (mat_Cube)
	{
		delete mat_Cube;
		mat_Cube = 0;
	}
	if (s_Texture)
		delete s_Texture;
}
void ResourcesClass::Initialize(HWND hwnd)
{
	InitializeMesh(hwnd);
	InitializeTexture(hwnd);
	InitializeShader(hwnd);
	InitializeMaterial(hwnd);


}

void ResourcesClass::InitializeMesh(HWND hwnd)
{
	m_GroundModel = new Mesh;
	m_GroundModel->Initialize(SystemClass::GetInstance()->GetDevice(), "../JHEngine/data/floor.txt");//�����
	m_CubeModel = new Mesh;
	m_CubeModel->Initialize(SystemClass::GetInstance()->GetDevice(), "../JHEngine/data/cube.txt");//ť��
}

void ResourcesClass::InitializeTexture(HWND hwnd)
{
	t_cube = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"../JHEngine/data/stone.dds");
	t_Floor = new TextureClass(SystemClass::GetInstance()->GetDevice(), L"../JHEngine/data/seafloor.dds");
}

void ResourcesClass::InitializeMaterial(HWND hwnd)
{
	MaterialClass* result = new MaterialClass;
	result->SetShader(s_Texture);
	result->AddTexture(t_cube);
	materialMap["cube"] = result;


	result = new MaterialClass;
	result->SetShader(s_Texture);
	result->AddTexture(t_Floor);
	materialMap["floor"] = result;
}

void ResourcesClass::InitializeShader(HWND hwnd)
{
	s_Texture = new TextureShaderClass;
	s_Texture->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
}

ResourcesClass * ResourcesClass::GetInstance()
{
	return SystemClass::GetInstance()->GetResources();
}
