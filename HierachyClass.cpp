#include "stdafx.h"
#include "HierachyClass.h"
#include "MeshRenderer.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "ResourcesClass.h"
#include "LightComponent.h"
#include "CameraComponent.h"
#include "InputComponent.h"

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
	// ground object ����
	GameObject* gobj = new GameObject("��");
	AddGameObject(gobj);
	MeshRenderer* renderer = new MeshRenderer;
	renderer->model = ResourcesClass::GetInstance()->m_GroundModel;
	gobj->AddComponent(renderer);
	gobj->position = XMFLOAT3(0, -1, 0);
	gobj->euler = XMFLOAT3(0, 20, 0);
	// cube object ����
	gobj = new GameObject("ť��");
	AddGameObject(gobj);
	renderer = new MeshRenderer;
	gobj->AddComponent(renderer);
	renderer->model = ResourcesClass::GetInstance()->m_CubeModel;
	gobj->AddComponent(new InputComponent);
	// light object ����
	gobj = new GameObject("����Ʈ");
	AddGameObject(gobj);
	LightComponent* m_Light = new LightComponent;
	gobj->AddComponent(m_Light);
	m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetPosition(2.0f, 5.0f, -2.0f);
	// light object ����
	gobj = new GameObject("ī�޶�");
	AddGameObject(gobj);
	CameraComponent* m_Camera = new CameraComponent;
	gobj->AddComponent(m_Camera);
	m_Camera->gameObject->position=(XMFLOAT3(0.0f, 7.0f, -10.0f));
	m_Camera->gameObject->euler=(XMFLOAT3(35.0f, 0.0f, 0.0f));
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