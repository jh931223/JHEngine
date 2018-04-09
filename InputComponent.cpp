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
	XMVECTOR v = XMVectorSet(axis.x, axis.y, axis.z, 1);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(Deg2Rad(gameObject->euler.x), Deg2Rad(gameObject->euler.y), Deg2Rad(gameObject->euler.z));

	XMMATRIX M;
	XMVECTOR V;

	M = XMMatrixRotationQuaternion(q);
	V = XMVectorMultiply(v, q);

	XMStoreFloat3(&axis, V);
	gameObject->position += axis;
}
