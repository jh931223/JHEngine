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
	// Ű �迭�� �ʱ�ȭ�մϴ�
	for (int i = 0; i<256; ++i)
	{
		m_keys[i] = false;
	}
	lastCursorPosition = GetCursorPosition();
}


void InputClass::KeyDown(unsigned int input)
{
	// Ű�� ���ȴٸ� �ش� Ű���� true�� �����մϴ�
	m_keys[input] = true;
}


void InputClass::KeyUp(unsigned int input)
{
	// Ű�� �����Ǿ��ٸ� �ش� Ű���� false�� �����մϴ�
	m_keys[input] = false;
}


bool InputClass::IsKeyDown(unsigned int key)
{
	// ���� Ű���� ���������� �ƴ��� ���¸� ��ȯ�մϴ�
	return m_keys[key];
}

void InputClass::MouseUpdate()
{
	lastCursorPosition = GetCursorPosition();
}

 XMFLOAT3 InputClass::GetCursorAxis()
{
	XMFLOAT3 deltaCursor = (GetCursorPosition() - lastCursorPosition);
	return XMFLOAT3(deltaCursor.x, deltaCursor.y,0)*0.1f;
}

 XMFLOAT3 InputClass::GetCursorPosition()
{
	POINT a;
	GetCursorPos(&a);
	return XMFLOAT3(a.y, a.x, 0);
}
