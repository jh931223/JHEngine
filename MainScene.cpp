#include "stdafx.h"
#include "MainScene.h"

#include "MeshClass.h"
#include "SystemClass.h"

#include "MeshRenderer.h"
#include "LightComponent.h"
#include "CameraComponent.h"
#include "InputComponent.h"
#include "VoxelComponent.h"
#include "BitmapRenderer.h"
#include "VoxelCollider.h"
using namespace std;


MainScene::MainScene()
{
}

void MainScene::Setup()
{
	GameObject* gobj;
	MeshRenderer* renderer;
	gobj = new GameObject("라이트");
	LightComponent* m_Light = gobj->AddComponent<LightComponent>();
	m_Light->SetAmbientColor(0.6f, 0.6f, 0.6f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetLookAt(238.3861f, 409.3519f, 524.0f);
	m_Light->transform()->SetPosition(XMFLOAT3(0.0f, 600.0f, 524.0f));
	m_Light->transform()->SetRotation(XMFLOAT3(38.0f, 90.0f, 0.0f));
	m_Light->GenerateViewMatrix();
	//m_Light->GeneratePerspectiveMatrix(1500, 1.0f);
	m_Light->GenerateOrthogrphicMatrix(1000.0f,2000, SHADOWMAP_NEAR);
	// light object 생성
	gobj = new GameObject("mainCamera");
	CameraComponent* m_Camera = gobj->AddComponent<CameraComponent>();
	m_Camera->transform()->SetPosition(XMFLOAT3(25.0f, 100.0f, -50.0f));
	m_Camera->transform()->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_Camera->background = CameraComponent::SkySphere;
	m_Camera->skyMaterial = ResourcesClass::GetInstance()->FindMaterial("m_skySphere");
	float screenAspect = (float)SystemClass::GetInstance()->GetWindowWidth() / (float)SystemClass::GetInstance()->GetWindowHeight();
	m_Camera->SetProjectionParameters((float)(XM_PI / 4.0f), screenAspect, 0.1f, 1500.0f);
	auto input=gobj->AddComponent<InputComponent>();
	// 복셀 생성
	gobj = new GameObject("복셀루트");
	GameObject* r = gobj;
	r->transform->SetPosition(XMFLOAT3(0, 0, 0));

	gobj = new GameObject("복셀");
	//r->AddChild(gobj);
	VoxelComponent* voxel = gobj->AddComponent<VoxelComponent>();
	voxel->brushInfo.brushType = VoxelComponent::BrushType::Brush_Sphere;
	/*VoxelComponent* voxel = new VoxelComponent;
	gobj->AddComponent(voxel);*/
	gobj->transform->SetPosition(XMFLOAT3(0, 0, 0));
	gobj->transform->SetRotation(XMFLOAT3(0, 0, 0));
	auto voxelCol = gobj->AddComponent<VoxelCollider>();
	voxelCol->voxel = voxel;

	gobj = new GameObject("큐브");
	//r->AddChild(gobj);
	renderer = gobj->AddComponent<MeshRenderer>();
	renderer->SetMesh(ResourcesClass::GetInstance()->FindMesh("cube"));
	renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_cube"));
	gobj->transform->SetPosition(XMFLOAT3(0, 0, 0));
	voxel->targetMesh = renderer;


	///CANVAS
	//{
	//	gobj = new GameObject("UI01");
	//	//r->AddChild(gobj);
	//	auto bRenderer=gobj->AddComponent<BitmapRenderer>();
	//	bRenderer->SetBitmap(255, 255);
	//	bRenderer->SetMaterial(SystemClass::GetInstance()->GetResources()->FindMaterial("m_texture"));
	//	gobj->transform->SetPosition(XMFLOAT3(0, 0, 0));
	//}

}


MainScene::~MainScene()
{
}
