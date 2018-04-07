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
	axis = XMFLOAT3(cos(Deg2Rad(gameObject->euler.y))*axis.x, 0, sin(Deg2Rad(gameObject->euler.y))*axis.z);
	gameObject->position += (Normalize3(axis)*0.1f);


	CameraComponent::mainCamera()->gameObject->euler += Input()->GetCursorAxis();// XMFLOAT3(0, 1, 0);
	CameraComponent::mainCamera()->gameObject->position = gameObject->position+XMFLOAT3(0,2,-5);
	//}
	//DebugMessage(""+(int)(Input()->GetCursorAxis().x));
}
