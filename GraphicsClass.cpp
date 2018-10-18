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


bool GraphicsClass::Initialize(int _screenWidth, int _screenHeight, HWND hwnd)
{
	// Direct3D 객체 생성
	m_Direct3D = new D3DClass;
	if(!m_Direct3D)
	{
		return false;
	}

	// Direct3D 객체 초기화
	bool result = m_Direct3D->Initialize(_screenWidth, _screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}


	// 뷰 포인트 객체를 만듭니다.
	//m_ViewPoint = new ViewPointClass;
	//if(!m_ViewPoint)
	//{
	//	return false;
	//}

	// 비트맵 객체 생성
	m_Bitmap = new BitmapClass;
	if (!m_Bitmap)
	{
		return false;
	}

	// 비트맵 객체 초기화
	if (!m_Bitmap->Initialize(m_Direct3D->GetDevice(), _screenWidth, _screenHeight, L"data/seafloor.dds",
		256, 256))
	{
		MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}
	// 뷰 포인트 객체를 초기화합니다.
	//m_ViewPoint->SetPosition(2.0f, 5.0f, -2.0f);
	//m_ViewPoint->SetLookAt(0.0f, 0.0f, 0.0f);
	//m_ViewPoint->SetProjectionParameters((float)(XM_PI / 2.0f), 1.0f, 0.1f, 100.0f);
	//m_ViewPoint->GenerateViewMatrix();
	//m_ViewPoint->GenerateProjectionMatrix();
	this->screenWidth = _screenWidth;
	this->screenHeight = _screenHeight;
	meshRenderers.clear();
	lights.clear();
	cameras.clear();
	return true;
}


void GraphicsClass::Shutdown()
{
	// 뷰 포인트 객체를 해제합니다.
	//if(m_ViewPoint)
	//{
	//	delete m_ViewPoint;
	//	m_ViewPoint = 0;
	//}
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
bool GraphicsClass::RenderScene(XMMATRIX viewMatrix, XMMATRIX projectionMatrix, Material* customMaterial)
{
	if (useMultiThreadedRendering)
	{
		if (!m_Direct3D->GetDeferredContextsSize())
			m_Direct3D->CreateDeferredContext(ITaskParallel::threadNums);
		RenderTask job;
		job.m_Direct3D = m_Direct3D;
		job.renderers = &meshRenderers;
		job.projectionMatrix = projectionMatrix;
		job.viewMatrix = viewMatrix;
		job.Schedule(meshRenderers.size(), 1);
		job.Dispatch();
		for (int i = 0; i < m_Direct3D->GetDeferredContextsSize(); i++)
		{
			if (m_Direct3D->GetCommandList(i))
				m_Direct3D->GetImmDeviceContext()->ExecuteCommandList(m_Direct3D->GetCommandList(i), FALSE);
		}
		//m_Direct3D->ReleaseDeferredContex();

	}
	else
	{
		//std::vector<MeshRenderer*> renderers = meshRenderers;
		for (const auto i : meshRenderers)
		{
			if (!i || !i->enabled)
			{
				continue;
			}
			//m_Direct3D->GetWorldMatrix(worldMatrix);
			GameObject* gameObject = i->gameObject;
			if (gameObject)
			{
				i->Render(m_Direct3D->GetImmDeviceContext(), viewMatrix, projectionMatrix);
			}
		}
		//printf("%d vertices \n", Frustum::drawnVertex);
	}
	return true;
}
bool GraphicsClass::RenderScene(CameraComponent* m_Camera,Material* customMaterial)
{
	if (m_Camera != 0)
		m_Camera->Render();
	XMMATRIX viewMatrix, projectionMatrix;
	m_Camera->GetViewMatrix(viewMatrix);
	m_Camera->GetProjectionMatrix(projectionMatrix);
	Frustum::ConstructFrustum(m_Camera->m_farPlane, projectionMatrix, viewMatrix);
	ShaderClass::SetRenderCam(m_Camera);
	return RenderScene(viewMatrix, projectionMatrix, customMaterial);
}

bool GraphicsClass::RenderCanvas(CameraComponent* m_Camera, Material* customMaterial)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	//m_Camera->GetViewMatrix(viewMatrix);
	CameraComponent::GetDefaultViewMtrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	// 모든 2D 렌더링을 시작하려면 Z 버퍼를 끕니다.
	//m_Direct3D->TurnZBufferOff();

	m_Direct3D->GetWorldMatrix(worldMatrix);
	// 비트 맵 버텍스와 인덱스 버퍼를 그래픽 파이프 라인에 배치하여 그리기를 준비합니다.
	if (!m_Bitmap->Render(m_Direct3D->GetImmDeviceContext(), 50, 100))
	{
		return false;
	}

	// 텍스처 쉐이더로 비트 맵을 렌더링합니다.    
	if (!ResourcesClass::GetInstance()->FindMaterial("cube")->Render(m_Direct3D->GetImmDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
	{
		return false;
	}

	// 모든 2D 렌더링이 완료되었으므로 Z 버퍼를 다시 켜십시오.
	//m_Direct3D->TurnZBufferOn();

	return true;
}

bool GraphicsClass::RenderToTexture(RenderTextureClass* m_RenderTexture, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, Material* customMaterial)
{
	m_RenderTexture->SetRenderTarget(m_Direct3D->GetImmDeviceContext());

	// 렌더링을 텍스처에 지웁니다.
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetImmDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);

	// 이제 장면을 렌더링하면 백 버퍼 대신 텍스처로 렌더링됩니다.
	if (!RenderScene(viewMatrix,projectionMatrix, customMaterial))
	{
		return false;
	}

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링에 대한 렌더링을 더 이상 다시 설정하지 않습니다.
	m_Direct3D->SetBackBufferRenderTarget();
	return true;
}
bool GraphicsClass::RenderToTexture(CameraComponent * m_Camera, Material* customMaterial)
{
	// 렌더링 대상을 렌더링에 맞게 설정합니다.
	RenderTextureClass* m_RenderTexture = m_Camera->GetRenderTexture();
	if (m_Camera != 0)
		m_Camera->Render();
	XMMATRIX viewMatrix, projectionMatrix;
	m_Camera->GetViewMatrix(viewMatrix);
	m_Camera->GetProjectionMatrix(projectionMatrix);
	Frustum::ConstructFrustum(m_Camera->m_farPlane, projectionMatrix, viewMatrix);
	ShaderClass::SetRenderCam(m_Camera);
	return RenderToTexture(m_RenderTexture, viewMatrix, projectionMatrix);
}

bool GraphicsClass::Render()
{

	CameraComponent* m_Camera = CameraComponent::mainCamera();
	LightComponent* m_Light = LightComponent::mainLight();

	////Render ShadowMap
	{
		if (!shadowMap)
		{
			shadowMap = new RenderTextureClass;
			if (!shadowMap)
			{
				return false;
			}
			if (!shadowMap->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR))
			{
				return false;
			}
			ResourcesClass::GetInstance()->AddResource("ShadowMap", shadowMap);
		}
		if (!shadowMapMaterial)
		{
			shadowMapMaterial = new Material;
			shadowMapMaterial->SetShader(ResourcesClass::GetInstance()->FindShader("DepthMapShader"));
			ResourcesClass::GetInstance()->AddResource("DepthMaterial", shadowMap);
		}
		XMMATRIX viewMatrix, projMatrix;
		m_Light->GetViewMatrix(viewMatrix);
		m_Light->GetProjectionMatrix(projMatrix);
		if (!RenderToTexture(shadowMap,viewMatrix,projMatrix, shadowMapMaterial))
			return false;
	}
	// 씬을 그리기 위해 버퍼를 지웁니다
	m_Direct3D->BeginScene(0.2f, 0.3f, 0.8f, 1.0f);
	// 카메라의 위치에 따라 뷰 행렬을 생성합니다
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
	
	 //렌더링 된 장면을 화면에 표시합니다.
	m_Direct3D->EndScene();

	return true;
}

bool RenderTask::Excute(int index)
{
	if (renderers->size() <= index)
		return false;
	int threadID = GetThreadID(index);
	auto i = (*renderers)[index];
	auto c = m_Direct3D->GetDeferredContext(threadID);
	if (!i)
		return false;
	GameObject* gameObject = i->gameObject;
	if (gameObject)
	{
		i->Render(c, viewMatrix, projectionMatrix);
	}
	return true;
}

void RenderTask::OnFinish(int threadID)
{
	auto c = m_Direct3D->GetDeferredContext(threadID);
	//printf("finish thread id : %d DC : %x\n", threadID,m_Direct3D->deferredContexts[threadID]);
	ID3D11CommandList*& cList = m_Direct3D->commandLists[threadID];// (m_Direct3D->GetCommandList(threadID));
	c->FinishCommandList(FALSE,  &cList);
}
