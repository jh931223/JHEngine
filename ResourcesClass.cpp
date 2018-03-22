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
{	// 큐브 모델 오브젝트를 해제하십시오.
	if (m_CubeModel)
	{
		m_CubeModel->Shutdown();
		delete m_CubeModel;
		m_CubeModel = 0;
	}

	// 그라운드 모델 객체를 해제한다.
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
	m_GroundModel->Initialize(SystemClass::GetInstance()->GetGraphics()->GetD3D()->GetDevice(), "../JHEngine/data/floor.txt", L"../JHEngine/data/stone.dds");//지면모델
	m_CubeModel = new Mesh;
	m_CubeModel->Initialize(SystemClass::GetInstance()->GetGraphics()->GetD3D()->GetDevice(), "../JHEngine/data/cube.txt", L"../JHEngine/data/seafloor.dds");//큐브
}

ResourcesClass * ResourcesClass::GetInstance()
{
	return SystemClass::GetInstance()->GetResources();
}
