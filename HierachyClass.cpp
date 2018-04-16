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
#include "MeshClass.h"
#include <map>
#include <string>
using namespace std;

HierachyClass::HierachyClass()
{
	Setup();
	Start();
}


HierachyClass::~HierachyClass()
{
	for (auto i : gameObjects)
		DestroyGameObject(i);
}

void HierachyClass::AddGameObject(GameObject * _gameObject)
{
	gameObjects.push_back(_gameObject);
}

void HierachyClass::DestroyGameObject(GameObject * _gameObject)
{
	std::vector<GameObject*>::iterator iter;
	for (iter=gameObjects.begin();iter!=gameObjects.end();iter++)
	{
		if (*iter == _gameObject)
		{
			gameObjects.erase(iter);
			break;
		}
	}
}


void HierachyClass::Setup()
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
	AddGameObject(gobj);
	LightComponent* m_Light = new LightComponent;
	gobj->AddComponent(m_Light);
	m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetLookAt(0.0f, 0.0f, 0.0f);
	m_Light->SetPosition(2.0f, 10.0f, -10.0f);
	m_Light->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
	m_Light->GenerateViewMatrix();
	// light object 생성
	gobj = new GameObject("카메라");
	AddGameObject(gobj);
	CameraComponent* m_Camera = new CameraComponent;
	gobj->AddComponent(m_Camera);
	m_Camera->gameObject->position=(XMFLOAT3(0.0f, 7.0f, -10.0f));
	m_Camera->gameObject->euler=(XMFLOAT3(35.0f, 0.0f, 0.0f));
	gobj->AddComponent(new InputComponent);
	// 복셀 생성
	gobj = new GameObject("복셀");
	AddGameObject(gobj);
	renderer = new MeshRenderer;
	gobj->AddComponent(renderer);
	Voxel* voxel = new Voxel;
	voxel->renderer = renderer;
	gobj->AddComponent(voxel);
	renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("test"));// 머테리얼 설정
	gobj->position = XMFLOAT3(0, -1, 0);
	gobj->euler = XMFLOAT3(0, 0, 0);
}

void HierachyClass::Start()
{
	for (auto iterObj : gameObjects)
		iterObj->OnStart();
}

void HierachyClass::Update()
{
	for (auto i : gameObjects)
	{
		i->Update();
	}
}