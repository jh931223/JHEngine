#include "stdafx.h"
#include "CameraComponent.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "BitmapClass.h"
#include "RenderTextureClass.h"
#include <list>

CameraComponent::CameraComponent()
{
	SystemClass::GetInstance()->GetGraphics()->PushCameras(this);
}


CameraComponent::~CameraComponent()
{
	SystemClass::GetInstance()->GetGraphics()->RemoveCameras(this);
}

void CameraComponent::ChangeDepth(int _depth)
{
	depth = _depth;
	SystemClass::GetInstance()->GetGraphics()->SortCameras();
}

void CameraComponent::Render()
{
	XMFLOAT3 position;
	XMVECTOR upVector, positionVector, lookAtVector;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;




	// XMVECTOR 구조체에 로드한다.
	upVector = XMVectorSet(0,1,0,0);

	// 3D월드에서 카메라의 위치를 ​​설정합니다.
	position = transform()->GetWorldPosition();

	// XMVECTOR 구조체에 로드한다.
	positionVector = XMLoadFloat3(&position);


	// XMVECTOR 구조체에 로드한다.
	lookAtVector = XMVectorSet(0,0,1,0);

	// yaw (Y 축), pitch (X 축) 및 roll (Z 축)의 회전값을 라디안 단위로 설정합니다.

	//  yaw, pitch, roll 값을 통해 회전 행렬을 만듭니다.
	rotationMatrix = transform()->GetRotateMatrix();

	// lookAt 및 up 벡터를 회전 행렬로 변형하여 뷰가 원점에서 올바르게 회전되도록 합니다.
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	// 회전 된 카메라 위치를 뷰어 위치로 변환합니다.
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	// 마지막으로 세 개의 업데이트 된 벡터에서 뷰 행렬을 만듭니다.
	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}


void CameraComponent::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void CameraComponent::GetProjectionMatrix(XMMATRIX &m_projectionMatrix)
{

	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);
}

void CameraComponent::GetDefaultViewMtrix(XMMATRIX& viewMatrix)
{
	viewMatrix = XMMatrixLookAtLH(XMVectorSet(0, 0, 0,0), XMVectorSet(0, 0, 1,0), XMVectorSet(0, 1, 0,0));
}

void CameraComponent::SetProjectionParameters(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
{
	m_fieldOfView = fieldOfView;
	m_aspectRatio = aspectRatio;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
}

RenderTextureClass * CameraComponent::GetRenderTexture()
{
	return renderTexture;
}

void CameraComponent::SetRenderTexture(RenderTextureClass * _bitmap)
{
	renderTexture->Shutdown();

}

CameraComponent* CameraComponent::mainCamera()
{
	return
		SystemClass::GetInstance()->GetGraphics()->GetMainCamera();
}
