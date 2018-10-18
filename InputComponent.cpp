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

	if (Input()->GetKeyDown(DIK_F))
	{
		Input()->ToggleMouseCursor(!Input()->GetMouseCursorToggle());
	}
	float deltaTime=Timer::DeltaTime();

	XMFLOAT3 euler = transform()->GetWorldRotation();
	XMFLOAT3 a = Input()->GetMouseAxis();
	float rSpeed =10.0f;
	transform()->SetRotation(euler + XMFLOAT3(a.y,a.x,0)*rSpeed*deltaTime);

	XMFLOAT3 axis(0, 0, 0);
	if (Input()->GetKey(DIK_D))
	{
		axis += BasicVector::right;
	}
	else if (Input()->GetKey(DIK_A))
	{
		axis -= BasicVector::right;
	}
	if (Input()->GetKey(DIK_W))
	{
		axis += BasicVector::forward;
	}
	else if (Input()->GetKey(DIK_S))
	{
		axis -= BasicVector::forward;
	}
	if (axis == XMFLOAT3(0, 0, 0))
		return;
	axis = Normalize3(axis);

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
}
