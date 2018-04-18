#include "stdafx.h"
#include "d3dclass.h"
#include "CameraComponent.h"
#include "meshclass.h"
#include "MaterialClass.h"
#include "LightComponent.h"
#include "textureclass.h"
#include "viewpointclass.h"
#include "graphicsclass.h"
#include "MeshRenderer.h"
#include "BitmapClass.h"
#include "ResourcesClass.h"
#include "RenderTextureClass.h"
GraphicsClass::GraphicsClass()
{
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	// Direct3D 객체 생성
	m_Direct3D = new D3DClass;
	if(!m_Direct3D)
	{
		return false;
	}

	// Direct3D 객체 초기화
	bool result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}



	// 뷰 포인트 객체를 만듭니다.
	m_ViewPoint = new ViewPointClass;
	if(!m_ViewPoint)
	{
		return false;
	}

	// 비트맵 객체 생성
	m_Bitmap = new BitmapClass;
	if (!m_Bitmap)
	{
		return false;
	}

	// 비트맵 객체 초기화
	if (!m_Bitmap->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, L"../JHEngine/data/seafloor.dds",
		256, 256))
	{
		MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}
	// 뷰 포인트 객체를 초기화합니다.
	m_ViewPoint->SetPosition(2.0f, 5.0f, -2.0f);
	m_ViewPoint->SetLookAt(0.0f, 0.0f, 0.0f);
	m_ViewPoint->SetProjectionParameters((float)(XM_PI / 2.0f), 1.0f, 0.1f, 100.0f);
	m_ViewPoint->GenerateViewMatrix();
	m_ViewPoint->GenerateProjectionMatrix();


	meshRenderers.clear();
	lights.clear();
	cameras.clear();
	return true;
}


void GraphicsClass::Shutdown()
{
	// 뷰 포인트 객체를 해제합니다.
	if(m_ViewPoint)
	{
		delete m_ViewPoint;
		m_ViewPoint = 0;
	}
	// m_Bitmap 객체 반환
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}


	// Direct3D 객체 반환
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}



}


bool GraphicsClass::Frame()
{
	// 그래픽 장면을 렌더링합니다.
	return Render();
}

D3DClass * GraphicsClass::GetD3D()
{
	return m_Direct3D;
}
bool GraphicsClass::RenderScene(CameraComponent* m_Camera,MaterialClass* customMaterial = nullptr)
{
	if (m_Camera != 0)
		m_Camera->Render();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 메쉬를 그립니다.
	for (const auto i : meshRenderers)
	{
		m_Direct3D->GetWorldMatrix(worldMatrix);
		GameObject* gameObject = i->gameObject;

		float pitch = gameObject->euler.x * 0.0174532925f;
		float yaw = gameObject->euler.y * 0.0174532925f;
		float roll = gameObject->euler.z * 0.0174532925f;

		//  yaw, pitch, roll 값을 통해 회전 행렬을 만듭니다.

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
		worldMatrix = XMMatrixTranslation(gameObject->position.x, gameObject->position.y, gameObject->position.z);
		i->GetMesh()->Render(m_Direct3D->GetDeviceContext());
		if (lights.size() == 0)
		{
			return false;
		}
		if (customMaterial != nullptr)
		{
			if (!customMaterial->Render(m_Direct3D->GetDeviceContext(), i->GetMesh()->GetIndexCount(), XMMatrixMultiply(rotationMatrix, worldMatrix), viewMatrix, projectionMatrix))
			{
				return false;
			}
		}
		else if (!i->GetMaterial()->Render(m_Direct3D->GetDeviceContext(), i->GetMesh()->GetIndexCount(), XMMatrixMultiply(rotationMatrix, worldMatrix), viewMatrix, projectionMatrix))
		{
			return false;
		}
	}
	return true;
}

bool GraphicsClass::RenderCanvas(CameraComponent* m_Camera)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	// 모든 2D 렌더링을 시작하려면 Z 버퍼를 끕니다.
	//m_Direct3D->TurnZBufferOff();

	m_Direct3D->GetWorldMatrix(worldMatrix);
	// 비트 맵 버텍스와 인덱스 버퍼를 그래픽 파이프 라인에 배치하여 그리기를 준비합니다.
	if (!m_Bitmap->Render(m_Direct3D->GetDeviceContext(), 50, 100))
	{
		return false;
	}

	// 텍스처 쉐이더로 비트 맵을 렌더링합니다.    
	if (!ResourcesClass::GetInstance()->FindMaterial("cube")->Render(m_Direct3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
	{
		return false;
	}

	// 모든 2D 렌더링이 완료되었으므로 Z 버퍼를 다시 켜십시오.
	//m_Direct3D->TurnZBufferOn();

	return true;
}

bool GraphicsClass::RenderToTexture(CameraComponent * camera)
{
	// 렌더링 대상을 렌더링에 맞게 설정합니다.
	RenderTextureClass* m_RenderTexture = ResourcesClass::GetInstance()->FindRenderTexture("rt_Shadow");

	m_RenderTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// 렌더링을 텍스처에 지웁니다.
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 1.0f,1.0f);

	// 이제 장면을 렌더링하면 백 버퍼 대신 텍스처로 렌더링됩니다.
	if (!RenderScene(camera,ResourcesClass::GetInstance()->FindMaterial("depthMap")))
	{
		return false;
	}

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링에 대한 렌더링을 더 이상 다시 설정하지 않습니다.
	m_Direct3D->SetBackBufferRenderTarget();

	return true;
}
bool GraphicsClass::RenderToDepthTexture(CameraComponent * camera)
{
	// 렌더링 대상을 렌더링에 맞게 설정합니다.
	RenderTextureClass* m_RenderTexture = ResourcesClass::GetInstance()->FindRenderTexture("rt_Shadow");

	m_RenderTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// 렌더링을 텍스처에 지웁니다.
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.3f, 0.5f, 1.0f);

	// 이제 장면을 렌더링하면 백 버퍼 대신 텍스처로 렌더링됩니다.
	if (!RenderScene(camera, ResourcesClass::GetInstance()->FindMaterial("depthMap")))
	{
		return false;
	}

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링에 대한 렌더링을 더 이상 다시 설정하지 않습니다.
	m_Direct3D->SetBackBufferRenderTarget();
	// 뷰포트를 원본으로 다시 설정합니다.
	m_Direct3D->ResetViewport();

	return true;
}
bool GraphicsClass::Render()
{

	CameraComponent* m_Camera = 0;
	if (cameras.size() > 0)
	{
		for (const auto i : cameras)
		{
			if (i->enabled)
			{
				m_Camera = i;
				break;
			}
		}
	}
	LightComponent* m_Light = 0;
	if (lights.size() > 0)
	{
		for (const auto i : lights)
		{
			if (i->enabled)
			{
				m_Light = i;
				break;
			}
		}
	}

	//if (!RenderToTexture(m_Camera))
	//	return false;

	// 씬을 그리기 위해 버퍼를 지웁니다
	m_Direct3D->BeginScene(0.0f, 0.3f, 0.8f, 1.0f);
	// 카메라의 위치에 따라 뷰 행렬을 생성합니다


	if (!RenderScene(m_Camera))
		return false;

	/*if (!RenderCanvas(m_Camera))
		return false;*/
	
	 //렌더링 된 장면을 화면에 표시합니다.
	m_Direct3D->EndScene();

	return true;
}