#include "stdafx.h"
#include "inputclass.h"
#include "SystemClass.h"

InputClass::InputClass()
{
	printf("inputClass 생성\n");
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
		m_keysPressing[i] = false;
	}
	centerOfScreen = XMFLOAT3((float)GetSystemMetrics(SM_CXSCREEN)/2, (float)GetSystemMetrics(SM_CYSCREEN)/2, 0);
	SetCursorPos(centerOfScreen.y, centerOfScreen.x);
	lastCursorPosition = GetCursorPosition();
	useCursor = false;
}


void InputClass::KeyDown(unsigned int input)
{
	// 키가 눌렸다면 해당 키값을 true로 저장합니다
	m_keys[input] = true;
	m_keysPressing[input] = false;
}


void InputClass::KeyUp(unsigned int input)
{
	// 키가 해제되었다면 해당 키값을 false로 저장합니다
	m_keys[input] = false;
	m_keysPressing[input] = false;
}


bool InputClass::GetKey(unsigned int key)
{
	// 현재 키값이 눌려졌는지 아닌지 상태를 반환합니다
	return m_keysPressing[key];
}
bool InputClass::GetKeyDown(unsigned int key)
{
	if (!m_keys[key])
		return false;
	if (m_keysPressing[key])
		return false;
	return true;
}

void InputClass::MouseUpdate()
{
	for (int i = 0; i < 256; i++)
	{
		if (m_keys[i])
		{
			m_keysPressing[i] = true;
		}
	}
	if(!useCursor)
		SetCursorPos(centerOfScreen.y, centerOfScreen.x);
}

void InputClass::ToggleCursor()
{
	useCursor = !useCursor;
}



 XMFLOAT3 InputClass::GetCursorAxis()
{
	 if (useCursor)
		 return XMFLOAT3(0, 0,0);
	XMFLOAT3 deltaCursor = (GetCursorPosition() - centerOfScreen);
	return XMFLOAT3(deltaCursor.x, deltaCursor.y,0)*0.1f;
}

 XMFLOAT3 InputClass::GetCursorPosition()
{
	POINT a;
	GetCursorPos(&a);

	return XMFLOAT3(a.y, a.x, 0);
}
