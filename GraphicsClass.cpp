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
	// Direct3D ��ü ����
	m_Direct3D = new D3DClass;
	if(!m_Direct3D)
	{
		return false;
	}

	// Direct3D ��ü �ʱ�ȭ
	bool result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}



	// �������� ���̴� ��ü�� ����ϴ�.
	m_ProjectionShader = new ProjectionShaderClass;
	if(!m_ProjectionShader)
	{
		return false;
	}

	// �������� ���̴� ��ü�� �ʱ�ȭ�մϴ�.
	result = m_ProjectionShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the projection shader object.", L"Error", MB_OK);
		return false;
	}

	// ���� �ؽ�ó ��ü�� ����ϴ�.
	m_ProjectionTexture = new TextureClass;
	if(!m_ProjectionTexture)
	{
		return false;
	}

	// ���� �ؽ�ó ��ü�� �ʱ�ȭ�մϴ�.
	result = m_ProjectionTexture->Initialize(m_Direct3D->GetDevice(), L"../JHEngine/data/grate.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the projection texture object.", L"Error", MB_OK);
		return false;
	}

	// �� ����Ʈ ��ü�� ����ϴ�.
	m_ViewPoint = new ViewPointClass;
	if(!m_ViewPoint)
	{
		return false;
	}

	// �� ����Ʈ ��ü�� �ʱ�ȭ�մϴ�.
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
	// �� ����Ʈ ��ü�� �����մϴ�.
	if(m_ViewPoint)
	{
		delete m_ViewPoint;
		m_ViewPoint = 0;
	}

	// ���� �ؽ�ó ��ü�� �����մϴ�.
	if(m_ProjectionTexture)
	{
		m_ProjectionTexture->Shutdown();
		delete m_ProjectionTexture;
		m_ProjectionTexture = 0;
	}

	// ���� ���̴� ��ü�� �����մϴ�.
	if(m_ProjectionShader)
	{
		m_ProjectionShader->Shutdown();
		delete m_ProjectionShader;
		m_ProjectionShader = 0;
	}

	// Direct3D ��ü ��ȯ
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}
}


bool GraphicsClass::Frame()
{
	// �׷��� ����� �������մϴ�.
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
	
	// ���� �׸��� ���� ���۸� ����ϴ�
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	// ī�޶��� ��ġ�� ���� �� ����� �����մϴ�
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

	// ī�޶� �� d3d ��ü���� ����, �� �� ���� ����� �����ɴϴ�
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// �� ����Ʈ ��ü���� �� �� ���� ����� �����ɴϴ�.
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

	// �޽��� �׸��ϴ�.

	for (const auto i:meshRenderers)
	{
		m_Direct3D->GetWorldMatrix(worldMatrix);
		GameObject* gameObject = i->gameObject;

		float pitch = gameObject->euler.x * 0.0174532925f;
		float yaw = gameObject->euler.y * 0.0174532925f;
		float roll = gameObject->euler.z * 0.0174532925f;

		//  yaw, pitch, roll ���� ���� ȸ�� ����� ����ϴ�.

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

							   
	 //������ �� ����� ȭ�鿡 ǥ���մϴ�.
	m_Direct3D->EndScene();

	return true;
}