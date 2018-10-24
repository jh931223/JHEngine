#include "stdafx.h"
#include "d3dclass.h"
#include "CameraComponent.h"
#include "meshclass.h"
#include "MaterialClass.h"
#include "LightComponent.h"
#include "textureclass.h"
#include "graphicsclass.h"
#include "MeshRenderer.h"
#include "BitmapRenderer.h"
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
void GraphicsClass::PushBitmapRenderer(BitmapRenderer* renderer)
{
	bitmapRenderers.push_back(renderer);
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
void GraphicsClass::RemoveBitmapRenderer(BitmapRenderer * renderer)
{
	for (int i = 0; i<bitmapRenderers.size(); i++)
		if (bitmapRenderers[i] == renderer)
		{
			bitmapRenderers.erase(bitmapRenderers.begin() + i);
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
XMFLOAT2 GraphicsClass::GetScreenSize()
{
	return XMFLOAT2(screenWidth, screenHeight);
}
ID3D11Device * GraphicsClass::GetDevice()
{
	return m_Direct3D->GetDevice();
}
bool GraphicsClass::RenderScene(XMMATRIX viewMatrix, XMMATRIX projectionMatrix, Material* customMaterial)
{

	// 하늘 출력
	auto renderCam = ShaderClass::GetRenderCam();
	if (customMaterial==NULL&&renderCam->background==CameraComponent::BackGroundType::SkySphere&&renderCam->skyMaterial)
	{
		m_Direct3D->TurnCullFront();
		m_Direct3D->TurnZBufferOff();
		XMMATRIX worldMatrix, skyProjectionMatrix;
		m_Direct3D->GetWorldMatrix(worldMatrix);
		m_Direct3D->GetProjectionMatrix(skyProjectionMatrix);
		float size = 4000.0f;
		worldMatrix = worldMatrix *XMMatrixScaling(size, size, size);
		auto _deviceContext = m_Direct3D->GetImmDeviceContext();
		if (!skyMesh)
			skyMesh = ResourcesClass::GetInstance()->FindMesh("sphere");
		skyMesh->Render(_deviceContext);
		renderCam->skyMaterial->Render(_deviceContext, worldMatrix, viewMatrix, skyProjectionMatrix);
		_deviceContext->DrawIndexed(skyMesh->GetIndexCount(), 0, 0);
		m_Direct3D->TurnCullBack();
		m_Direct3D->TurnZBufferOn();
	}
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
				i->Render(m_Direct3D->GetImmDeviceContext(), viewMatrix, projectionMatrix, customMaterial);
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
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Camera->Render();
	m_Camera->GetViewMtrixForCanvas(viewMatrix);
	//CameraComponent::GetDefaultViewMtrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(projectionMatrix);
	// 모든 2D 렌더링을 시작하려면 Z 버퍼를 끕니다.
	m_Direct3D->TurnZBufferOff();
	for (const auto i : bitmapRenderers)
	{
		if (!i || !i->enabled)
		{
			continue;
		}
		GameObject* gameObject = i->gameObject;
		if (gameObject)
		{
			i->Render(m_Direct3D->GetImmDeviceContext(), viewMatrix, projectionMatrix);
		}
	}
	// 모든 2D 렌더링이 완료되었으므로 Z 버퍼를 다시 켜십시오.
	m_Direct3D->TurnZBufferOn();

	return true;
}

bool GraphicsClass::RenderToTexture(RenderTextureClass* m_RenderTexture, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, Material* customMaterial)
{
	m_RenderTexture->SetRenderTarget(m_Direct3D->GetImmDeviceContext());

	// 렌더링을 텍스처에 지웁니다.
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetImmDeviceContext(),1.0f, 1.0f,1.0f, 1.0f);

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

	/// 그림자패스
	{
		Frustum::ToggleFrustumCulling(false);
		if (!shadowMap)
			shadowMap = ResourcesClass::GetInstance()->FindRenderTexture("ShadowMap");
		if (!shadowMapMaterial)
			shadowMapMaterial = ResourcesClass::GetInstance()->FindMaterial("m_depthMap");
		XMMATRIX viewMatrix, projMatrix;
		//m_Camera->Render();
		m_Direct3D->TurnCullOff();
		m_Light->GetViewMatrix(viewMatrix);
		m_Light->GetProjectionMatrix(projMatrix);
		if (!RenderToTexture(shadowMap,viewMatrix,projMatrix, shadowMapMaterial))
			return false;
		m_Direct3D->TurnCullBack();
		Frustum::ToggleFrustumCulling(true);
	}

	XMFLOAT4 backgroundColor = m_Camera->backGroundColor;
	m_Direct3D->BeginScene(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);

	for (auto i : cameras)
	{
		if (i->GetRenderTexture())
			RenderToTexture(i);
		else
			RenderScene(i);
	}

	RenderCanvas(m_Camera);
	
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
	c->FinishCommandList(FALSE, &cList);
}
