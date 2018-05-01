#include "stdafx.h"
#include "InputComponent.h"
#include "InputClass.h"
#include "SystemClass.h"
#include "CameraComponent.h"
InputComponent::InputComponent()
{
}


InputComponent::~InputComponent()
{
}

void InputComponent::Update()
{
	if (Input()->IsKeyDown(VK_CONTROL))
	{
		Input()->ToggleCursor();
	}

	XMFLOAT3 axis(0,0,0);
	if (Input()->IsKeyDown(68))
	{
		axis += right();
	}
	else if (Input()->IsKeyDown(65))
	{
		axis -= right();
	}
	if (Input()->IsKeyDown(87))
	{
		axis += forward();
	}
	else if (Input()->IsKeyDown(83))
	{
		axis -= forward();
	}
	axis=Normalize3(axis);


	XMFLOAT3 euler = transform()->GetWorldRotation();
	transform()->SetRotation(euler + Input()->GetCursorAxis());
	XMVECTOR v = XMVectorSet(axis.x, axis.y, axis.z, 0.0f);

	XMVECTOR q = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(euler.x), XMConvertToRadians(euler.y), XMConvertToRadians(euler.z));
	XMMATRIX m=XMMatrixRotationQuaternion(q);
	v = XMVector3Transform(v, m);
	XMStoreFloat3(&axis, v);
	transform()->TranslateW(axis*0.1f);
}
