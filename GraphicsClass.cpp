#include "stdafx.h"
#include "d3dclass.h"
#include "CameraComponent.h"
#include "meshclass.h"
#include "LightComponent.h"
#include "projectionshaderclass.h"
#include "textureclass.h"
#include "viewpointclass.h"
#include "graphicsclass.h"
#include "MeshRenderer.h"

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



	// 프로젝션 셰이더 개체를 만듭니다.
	m_ProjectionShader = new ProjectionShaderClass;
	if(!m_ProjectionShader)
	{
		return false;
	}

	// 프로젝션 셰이더 개체를 초기화합니다.
	result = m_ProjectionShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the projection shader object.", L"Error", MB_OK);
		return false;
	}

	// 투영 텍스처 객체를 만듭니다.
	m_ProjectionTexture = new TextureClass;
	if(!m_ProjectionTexture)
	{
		return false;
	}

	// 투영 텍스처 객체를 초기화합니다.
	result = m_ProjectionTexture->Initialize(m_Direct3D->GetDevice(), L"../JHEngine/data/grate.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the projection texture object.", L"Error", MB_OK);
		return false;
	}


	// 뷰 포인트 객체를 만듭니다.
	m_ViewPoint = new ViewPointClass;
	if(!m_ViewPoint)
	{
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

	// 투영 텍스처 객체를 해제합니다.
	if(m_ProjectionTexture)
	{
		m_ProjectionTexture->Shutdown();
		delete m_ProjectionTexture;
		m_ProjectionTexture = 0;
	}


	// 투영 쉐이더 객체를 해제합니다.
	if(m_ProjectionShader)
	{
		m_ProjectionShader->Shutdown();
		delete m_ProjectionShader;
		m_ProjectionShader = 0;
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


bool GraphicsClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMMATRIX viewMatrix2, projectionMatrix2;
	
	// 씬을 그리기 위해 버퍼를 지웁니다
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	// 카메라의 위치에 따라 뷰 행렬을 생성합니다
	CameraComponent* m_Camera=0;
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
	if(m_Camera!=0)
		m_Camera->Render();

	// 카메라 및 d3d 객체에서 월드, 뷰 및 투영 행렬을 가져옵니다
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 뷰 포인트 객체에서 뷰 및 투영 행렬을 가져옵니다.
	m_ViewPoint->GetViewMatrix(viewMatrix2);
	m_ViewPoint->GetProjectionMatrix(projectionMatrix2);

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

	// 메쉬를 그립니다.

	for (const auto i:meshRenderers)
	{
		m_Direct3D->GetWorldMatrix(worldMatrix);
		GameObject* gameObject = i->gameObject;

		float pitch = gameObject->euler.x * 0.0174532925f;
		float yaw = gameObject->euler.y * 0.0174532925f;
		float roll = gameObject->euler.z * 0.0174532925f;

		//  yaw, pitch, roll 값을 통해 회전 행렬을 만듭니다.

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
		worldMatrix = XMMatrixTranslation(gameObject->position.x, gameObject->position.y, gameObject->position.z);
		i->model->Render(m_Direct3D->GetDeviceContext());
		if (lights.size() == 0)
		{
			return false;
		}
		if (!m_ProjectionShader->Render(m_Direct3D->GetDeviceContext(), i->model->GetIndexCount(), rotationMatrix*worldMatrix, viewMatrix, projectionMatrix,
			i->model->GetTexture(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetPosition(),
			viewMatrix2, projectionMatrix2, m_ProjectionTexture->GetTexture()))
		{
			return false;
		}
	}

							   
	 //렌더링 된 장면을 화면에 표시합니다.
	m_Direct3D->EndScene();

	return true;
}