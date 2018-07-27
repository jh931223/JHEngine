#pragma once
#include "stdafx.h"
#include "HierachyClass.h"
#include "MeshRenderer.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "ResourcesClass.h"
#include "LightComponent.h"
#include "CameraComponent.h"
#include "InputComponent.h"
#include "VoxelComponent.h"
#include "A_VoxelComponent.h"
#include "MeshClass.h"
#include <map>
#include <string>
using namespace std;

HierarchyClass::HierarchyClass()
{
}


HierarchyClass::~HierarchyClass()
{
	for (auto i : gameObjects)
		DestroyGameObject(i);
}

void HierarchyClass::AddGameObject(GameObject * _gameObject)
{
	gameObjects.push_back(_gameObject);
	newGameObjects.push_back(_gameObject);
}

void HierarchyClass::DestroyGameObject(GameObject * _gameObject)
{
	printf("%s 삭제됨\n", _gameObject->name);
	delete _gameObject;
	auto g1 = std::find(gameObjects.begin(), gameObjects.end(), _gameObject);
	if (g1 != gameObjects.end())
		gameObjects.erase(g1);
	auto g = std::find(newGameObjects.begin(), newGameObjects.end(), _gameObject);
	if (g != newGameObjects.end())
		newGameObjects.erase(g);
	/*for (iter = newGameObjects.begin(); iter != newGameObjects.end(); iter++)
	{
		if (*iter == _gameObject)
		{
			newGameObjects.erase(iter);
			break;
		}
	}*/
}


void HierarchyClass::Setup()
{
	GameObject* gobj;
	MeshRenderer* renderer;
	// ground object 생성
	//gobj = new GameObject("땅");
	//AddGameObject(gobj);
	//renderer = new MeshRenderer;
	//renderer->SetMesh(ResourcesClass::GetInstance()->FindMesh("floor"));//메쉬 설정
	//renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("test"));//머테리얼 설정
	//gobj->AddComponent(renderer);
	//gobj->position = XMFLOAT3(0, -1, 0);
	//gobj->euler = XMFLOAT3(0, 0, 0);
	//// cube object 생성
	//gobj = new GameObject("큐브");
	//AddGameObject(gobj);
	//renderer = new MeshRenderer;
	//gobj->AddComponent(renderer);
	//renderer->SetMesh(ResourcesClass::GetInstance()->FindMesh("cube"));// 메쉬 설정
	//renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("cube"));// 머테리얼 설정
	// light object 생성
	gobj = new GameObject("라이트");
	LightComponent* m_Light = new LightComponent;
	gobj->AddComponent(m_Light);
	m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetLookAt(0.0f, 0.0f, 0.0f);
	m_Light->transform()->SetPosition(XMFLOAT3(2.0f, 10.0f, -10.0f));
	m_Light->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
	m_Light->GenerateViewMatrix();
	// light object 생성
	gobj = new GameObject("mainCamera");
	CameraComponent* m_Camera = new CameraComponent;
	gobj->AddComponent(m_Camera);
	m_Camera->transform()->SetPosition(XMFLOAT3(25.0f, 0.0f, -50.0f));
	m_Camera->transform()->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
	float screenAspect = (float)SystemClass::GetInstance()->GetScreenWidth() / (float)SystemClass::GetInstance()->GetScreenHeight();
	m_Camera->SetProjectionParameters((float)(XM_PI / 4.0f), screenAspect, 0.1f, 1000.0f);
	gobj->AddComponent(new InputComponent);
	// 복셀 생성
	gobj = new GameObject("복셀루트");
	GameObject* r = gobj;
	r->transform->SetPosition(XMFLOAT3(0, 0, 0));

	gobj = new GameObject("복셀");
	//r->AddChild(gobj);
	renderer = new MeshRenderer;
	gobj->AddComponent(renderer);
	A_VoxelComponent* voxel = new A_VoxelComponent;
	gobj->AddComponent(voxel);
	gobj->transform->SetPosition(XMFLOAT3(0, 0, 0));
	gobj->transform->SetRotation(XMFLOAT3(0, 0, 0));
	
}

void HierarchyClass::Start()
{
	for (auto iterObj : newGameObjects)
	{
		if(iterObj)
			iterObj->OnStart();
	}
	newGameObjects.clear();
}

void HierarchyClass::Update()
{
	vector<GameObject*> gObjects = gameObjects;
	vector<GameObject*> newGObjects = newGameObjects;
	newGameObjects.clear();
	if (newGObjects.size())
	{
		for (auto i : newGObjects)
		{
			i->OnStart();
		}
		newGObjects.clear();
	}
	for (auto i : gObjects)
	{
		i->Update();
	}
	for (auto i : gObjects)
	{
		i->LateUpdate();
	}
}