#include "stdafx.h"
#include "InputComponent.h"
#include "InputClass.h"
#include "SystemClass.h"
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
		axis += up();
	}
	else if (Input()->IsKeyDown(83))
	{
		axis -= up();
	}
	axis.z = axis.y;
	axis.y = 0;
	gameObject->position += (Normalize3(axis)*0.1f);
	gameObject->euler += Input()->GetCursorAxis();// XMFLOAT3(0, 1, 0);
	//}
	//DebugMessage(""+(int)(Input()->GetCursorAxis().x));
}
