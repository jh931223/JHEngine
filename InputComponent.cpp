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


	gameObject->euler += Input()->GetCursorAxis();
	XMVECTOR v = XMVectorSet(axis.x, axis.y, axis.z, 0.0f);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(gameObject->euler.x), XMConvertToRadians(gameObject->euler.y), XMConvertToRadians(gameObject->euler.z));
	XMMATRIX m=XMMatrixRotationQuaternion(q);
	v = XMVector3Transform(v, m);
	XMStoreFloat3(&axis, v);
	gameObject->position += axis*0.1f;
}
