#include "stdafx.h"
#include "LightComponent.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include <list>


std::list<LightComponent*> LightComponent::lights;

LightComponent::LightComponent()
{
	lights.push_back(this);
	//SystemClass::GetInstance()->GetGraphics()->PushLights(this);
}



LightComponent::~LightComponent()
{
	for(auto i=lights.begin();i!=lights.end();i++)
		if (*i == this)
		{
			lights.erase(i);
			break;
		}
	//SystemClass::GetInstance()->GetGraphics()->RemoveLights(this);
}


void LightComponent::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = XMFLOAT4(red, green, blue, alpha);
}


void LightComponent::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
}



void LightComponent::SetLookAt(float x, float y, float z)
{
	m_lookAt = XMFLOAT3(x, y, z);
}


XMFLOAT4 LightComponent::GetAmbientColor()
{
	return m_ambientColor;
}


XMFLOAT4 LightComponent::GetDiffuseColor()
{
	return m_diffuseColor;
}




void LightComponent::GenerateViewMatrix()
{
	// ������ ����Ű�� ���͸� �����մϴ�.
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	XMVECTOR upVector = XMLoadFloat3(&up);
	XMVECTOR positionVector = XMLoadFloat3(&(transform()->GetWorldPosition()));
	XMVECTOR lookAtVector = XMLoadFloat3(&m_lookAt);

	// �� ���ͷκ��� �� ����� ����ϴ�.
	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}


void LightComponent::GenerateProjectionMatrix(float screenDepth, float screenNear)
{
	// ���簢�� ������ ���� �þ� �� ȭ�� ������ �����մϴ�.
	float fieldOfView = (float)XM_PI / 2.0f;
	float screenAspect = 1.0f;

	// ���� ���� ����� ����ϴ�.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
}


void LightComponent::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}


void LightComponent::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}

LightComponent * LightComponent::mainLight()
{
	return lights.front();
}

void LightComponent::SetPosition(float x, float y, float z)
{
	transform()->SetPosition(XMFLOAT3(x, y, z));
}

XMFLOAT3 LightComponent::GetPosition()
{
	return transform()->GetWorldPosition();
}