#include "stdafx.h"
#include "ResourcesClass.h"
#include "MeshClass.h"
#include "TextureClass.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "d3dclass.h"
ResourcesClass::ResourcesClass()
{
	Initialize();
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
}

void ResourcesClass::Initialize()
{
	m_GroundModel = new Mesh;
	m_GroundModel->Initialize(SystemClass::GetInstance()->GetGraphics()->GetD3D()->GetDevice(), "../JHEngine/data/floor.txt", L"../JHEngine/data/stone.dds");//�����
	m_CubeModel = new Mesh;
	m_CubeModel->Initialize(SystemClass::GetInstance()->GetGraphics()->GetD3D()->GetDevice(), "../JHEngine/data/cube.txt", L"../JHEngine/data/seafloor.dds");//ť��
}

ResourcesClass * ResourcesClass::GetInstance()
{
	return SystemClass::GetInstance()->GetResources();
}
