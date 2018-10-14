#include "stdafx.h"
#include "InputComponent.h"
#include "InputClass.h"
#include "Timer.h"
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
	if (Input()->GetKey(DIK_D))
	{
		axis += right();
	}
	else if (Input()->GetKey(DIK_A))
	{
		axis -= right();
	}
	if (Input()->GetKey(DIK_W))
	{
		axis += forward();
	}
	else if (Input()->GetKey(DIK_S))
	{
		axis -= forward();
	}
	axis=Normalize3(axis);
	float deltaTime=Timer::DeltaTime();

	XMFLOAT3 euler = transform()->GetWorldRotation();
	XMFLOAT3 a = Input()->GetMouseAxis();
	float rSpeed =10.0f;
	transform()->SetRotation(euler + XMFLOAT3(a.y,a.x,0)*rSpeed*deltaTime);
	XMVECTOR v = XMVectorSet(axis.x, axis.y, axis.z, 0.0f);

	XMVECTOR q = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(euler.x), XMConvertToRadians(euler.y), XMConvertToRadians(euler.z));
	XMMATRIX m=XMMatrixRotationQuaternion(q);
	v = XMVector3Transform(v, m);
	XMStoreFloat3(&axis, v);
	float speed = 20.0;
	if (Input()->GetKey(DIK_LSHIFT))
		speed = 30.0f;
	else if (Input()->GetKey(DIK_LCONTROL))
		speed = 2.0f;
	transform()->TranslateW(axis*speed*deltaTime);
	if (Input()->GetKeyDown(DIK_F))
	{
		Input()->ToggleMouseCursor(!Input()->GetMouseCursorToggle());
	}
}
