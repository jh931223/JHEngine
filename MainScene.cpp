#include "stdafx.h"
#include "MainScene.h"

#include "MeshRenderer.h"
#include "LightComponent.h"
#include "CameraComponent.h"
#include "InputComponent.h"
#include "VoxelComponent.h"
#include "MeshClass.h"
using namespace std;


MainScene::MainScene()
{
}

void MainScene::Setup()
{
	GameObject* gobj;
	MeshRenderer* renderer;
	gobj = new GameObject("¶óÀÌÆ®");
	LightComponent* m_Light = new LightComponent;
	gobj->AddComponent(m_Light);
	m_Light->SetAmbientColor(0.1f, 0.1f, 0.1f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetLookAt(0.0f, 0.0f, 0.0f);
	m_Light->transform()->SetPosition(XMFLOAT3(2.0f, 10.0f, -10.0f));
	m_Light->GenerateViewMatrix();
	m_Light->GenerateOrthogrphicMatrix(20.0f,SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
	// light object »ý¼º
	gobj = new GameObject("mainCamera");
	CameraComponent* m_Camera = new CameraComponent;
	gobj->AddComponent(m_Camera);
	m_Camera->transform()->SetPosition(XMFLOAT3(25.0f, 0.0f, -50.0f));
	m_Camera->transform()->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
	float screenAspect = (float)SystemClass::GetInstance()->GetScreenWidth() / (float)SystemClass::GetInstance()->GetScreenHeight();
	m_Camera->SetProjectionParameters((float)(XM_PI / 4.0f), screenAspect, 0.1f, 1500.0f);
	gobj->AddComponent(new InputComponent);
	// º¹¼¿ »ý¼º
	gobj = new GameObject("º¹¼¿·çÆ®");
	GameObject* r = gobj;
	r->transform->SetPosition(XMFLOAT3(0, 0, 0));

	gobj = new GameObject("º¹¼¿");
	//r->AddChild(gobj);
	renderer = new MeshRenderer;
	gobj->AddComponent(renderer);
	VoxelComponent* voxel = new VoxelComponent;
	gobj->AddComponent(voxel);
	gobj->transform->SetPosition(XMFLOAT3(0, 0, 0));
	gobj->transform->SetRotation(XMFLOAT3(0, 0, 0));
}


MainScene::~MainScene()
{
}
