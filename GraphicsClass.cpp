#include "stdafx.h"
#include "d3dclass.h"
#include "CameraComponent.h"
#include "meshclass.h"
#include "MaterialClass.h"
#include "LightComponent.h"
#include "textureclass.h"
#include "graphicsclass.h"
#include "MeshRenderer.h"
#include "BitmapClass.h"
#include "ResourcesClass.h"
#include "RenderTextureClass.h"
#include"Transform.h"
#include "SceneClass.h"
#include <algorithm>
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


	// �� ����Ʈ ��ü�� ����ϴ�.
	//m_ViewPoint = new ViewPointClass;
	//if(!m_ViewPoint)
	//{
	//	return false;
	//}

	// ��Ʈ�� ��ü ����
	m_Bitmap = new BitmapClass;
	if (!m_Bitmap)
	{
		return false;
	}

	// ��Ʈ�� ��ü �ʱ�ȭ
	if (!m_Bitmap->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, L"data/seafloor.dds",
		256, 256))
	{
		MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}
	// �� ����Ʈ ��ü�� �ʱ�ȭ�մϴ�.
	//m_ViewPoint->SetPosition(2.0f, 5.0f, -2.0f);
	//m_ViewPoint->SetLookAt(0.0f, 0.0f, 0.0f);
	//m_ViewPoint->SetProjectionParameters((float)(XM_PI / 2.0f), 1.0f, 0.1f, 100.0f);
	//m_ViewPoint->GenerateViewMatrix();
	//m_ViewPoint->GenerateProjectionMatrix();


	meshRenderers.clear();
	lights.clear();
	cameras.clear();
	return true;
}


void GraphicsClass::Shutdown()
{
	// �� ����Ʈ ��ü�� �����մϴ�.
	//if(m_ViewPoint)
	//{
	//	delete m_ViewPoint;
	//	m_ViewPoint = 0;
	//}
	// m_Bitmap ��ü ��ȯ
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
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
	m_Direct3D->ExcuteCommandLists();
	return Render();
}

D3DClass * GraphicsClass::GetD3D()
{
	return m_Direct3D;
}
void GraphicsClass::PushRenderer(MeshRenderer * renderer)
{
	meshRenderers.push_back(renderer);
}
void GraphicsClass::PushLights(LightComponent * light)
{
	lights.push_back(light);
}
void GraphicsClass::PushCameras(CameraComponent * cam)
{
	cameras.push_back(cam);
	SortCameras();
}
void GraphicsClass::SortCameras()
{
	std::sort(cameras.begin(), cameras.end(), [](CameraComponent* lhs, CameraComponent* rhs) {return lhs->depth<rhs->depth; });
}
void GraphicsClass::RemoveRenderer(MeshRenderer * renderer)
{
	for(int i=0;i<meshRenderers.size();i++)
		if (meshRenderers[i] == renderer)
		{
			meshRenderers.erase(meshRenderers.begin() + i);
			break;
		}
}
void GraphicsClass::RemoveLights(LightComponent * light)
{
	for (int i = 0; i<lights.size(); i++)
		if (lights[i] == light)
		{
			lights.erase(lights.begin() + i);
			break;
		}
}
void GraphicsClass::RemoveCameras(CameraComponent * cam)
{
	for (int i = 0; i<cameras.size(); i++)
		if (cameras[i] == cam)
		{
			cameras.erase(cameras.begin() + i);
			break;
		}
}
CameraComponent * GraphicsClass::GetMainCamera()
{
	CameraComponent* top=NULL;
	if (cameras.size())
		top = cameras[0];
	return top;
}
bool GraphicsClass::RenderScene(CameraComponent* m_Camera,Material* customMaterial = nullptr)
{
	if (m_Camera != 0)
		m_Camera->Render();
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Camera->GetProjectionMatrix(projectionMatrix);
	std::vector<MeshRenderer*> renderers = meshRenderers;
	// �޽��� �׸��ϴ�.
	for (const auto i : renderers)
	{
		if (!i)
		{
			continue;
		}
		m_Direct3D->GetWorldMatrix(worldMatrix);
		GameObject* gameObject = i->gameObject;
		if (gameObject)
		{
			//  yaw, pitch, roll ���� ���� ȸ�� ����� ����ϴ�.
			i->Render(m_Direct3D->GetImmDeviceContext(), gameObject->transform->GetTransformMatrix(), viewMatrix, projectionMatrix);
		}
	}
	return true;
}

bool GraphicsClass::RenderCanvas(CameraComponent* m_Camera)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	//m_Camera->GetViewMatrix(viewMatrix);
	CameraComponent::GetDefaultViewMtrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	// ��� 2D �������� �����Ϸ��� Z ���۸� ���ϴ�.
	//m_Direct3D->TurnZBufferOff();

	m_Direct3D->GetWorldMatrix(worldMatrix);
	// ��Ʈ �� ���ؽ��� �ε��� ���۸� �׷��� ������ ���ο� ��ġ�Ͽ� �׸��⸦ �غ��մϴ�.
	if (!m_Bitmap->Render(m_Direct3D->GetImmDeviceContext(), 50, 100))
	{
		return false;
	}

	// �ؽ�ó ���̴��� ��Ʈ ���� �������մϴ�.    
	if (!ResourcesClass::GetInstance()->FindMaterial("cube")->Render(m_Direct3D->GetImmDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
	{
		return false;
	}

	// ��� 2D �������� �Ϸ�Ǿ����Ƿ� Z ���۸� �ٽ� �ѽʽÿ�.
	//m_Direct3D->TurnZBufferOn();

	return true;
}

bool GraphicsClass::RenderToTexture(CameraComponent * camera)
{
	// ������ ����� �������� �°� �����մϴ�.
	RenderTextureClass* m_RenderTexture = camera->GetRenderTexture();

	m_RenderTexture->SetRenderTarget(m_Direct3D->GetImmDeviceContext());

	// �������� �ؽ�ó�� ����ϴ�.
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetImmDeviceContext(), 0.0f, 0.0f, 1.0f,1.0f);

	// ���� ����� �������ϸ� �� ���� ��� �ؽ�ó�� �������˴ϴ�.
	if (!RenderScene(camera))
	{
		return false;
	}

	// ������ ����� ������ �� ���۷� �ٽ� �����ϰ� �������� ���� �������� �� �̻� �ٽ� �������� �ʽ��ϴ�.
	m_Direct3D->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::Render()
{

	CameraComponent* m_Camera = CameraComponent::mainCamera();
	LightComponent* m_Light = LightComponent::mainLight();

	//if (!RenderToTexture(m_Camera))
	//	return false;

	// ���� �׸��� ���� ���۸� ����ϴ�
	m_Direct3D->BeginScene(0.2f, 0.3f, 0.8f, 1.0f);
	// ī�޶��� ��ġ�� ���� �� ����� �����մϴ�
	//if (!RenderScene(m_Camera))
	//	return false;
	for (auto i : cameras)
	{
		if (i->GetRenderTexture())
			RenderToTexture(i);
		else
			RenderScene(i);
	}

	//if (!RenderCanvas(m_Camera))
	//	return false;
	
	 //������ �� ����� ȭ�鿡 ǥ���մϴ�.
	m_Direct3D->EndScene();

	return true;
}