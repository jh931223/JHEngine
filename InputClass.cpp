#include "stdafx.h"
#include "inputclass.h"
#include "SystemClass.h"

InputClass::InputClass()
{
}


InputClass::~InputClass()
{
}


void InputClass::Initialize()
{
	// 키 배열을 초기화합니다
	for (int i = 0; i<256; ++i)
	{
		m_keys[i] = false;
	}
	centerOfScrren = XMFLOAT3((float)GetSystemMetrics(SM_CXSCREEN)/2, (float)GetSystemMetrics(SM_CYSCREEN)/2, 0);
	SetCursorPos(centerOfScrren.y, centerOfScrren.x);
	lastCursorPosition = GetCursorPosition();
}


void InputClass::KeyDown(unsigned int input)
{
	// 키가 눌렸다면 해당 키값을 true로 저장합니다
	m_keys[input] = true;
}


void InputClass::KeyUp(unsigned int input)
{
	// 키가 해제되었다면 해당 키값을 false로 저장합니다
	m_keys[input] = false;
}


bool InputClass::IsKeyDown(unsigned int key)
{
	// 현재 키값이 눌려졌는지 아닌지 상태를 반환합니다
	return m_keys[key];
}

void InputClass::MouseUpdate()
{
	SetCursorPos(centerOfScrren.y, centerOfScrren.x);
}

 XMFLOAT3 InputClass::GetCursorAxis()
{
	XMFLOAT3 deltaCursor = (GetCursorPosition() - centerOfScrren);
	return XMFLOAT3(deltaCursor.x, deltaCursor.y,0)*0.1f;
}

 XMFLOAT3 InputClass::GetCursorPosition()
{
	POINT a;
	GetCursorPos(&a);

	return XMFLOAT3(a.y, a.x, 0);
}
