#include "stdafx.h"
#include "LightComponent.h"
#include "SystemClass.h"
#include "GraphicsClass.h"

LightComponent::LightComponent()
{
	SystemClass::GetInstance()->GetGraphics()->lights.push_back(this);
}



LightComponent::~LightComponent()
{
	SystemClass::GetInstance()->GetGraphics()->lights.remove(this);
}


void LightComponent::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = XMFLOAT4(red, green, blue, alpha);
}


void LightComponent::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
}


void LightComponent::SetPosition(float x, float y, float z)
{
	gameObject->position = XMFLOAT3(x, y, z);
}


XMFLOAT4 LightComponent::GetAmbientColor()
{
	return m_ambientColor;
}


XMFLOAT4 LightComponent::GetDiffuseColor()
{
	return m_diffuseColor;
}


XMFLOAT3 LightComponent::GetPosition()
{
	return gameObject->position;
}
