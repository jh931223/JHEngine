#include "stdafx.h"
#include "inputclass.h"
#include "SystemClass.h"

InputClass::InputClass()
{
}


InputClass::~InputClass()
{
}


bool InputClass::Initialize(HINSTANCE hinstance, HWND hwnd)
{


	// 마우스 커서의 위치 지정에 사용될 화면 크기를 설정합니다.

	// Direct Input 인터페이스를 초기화 합니다.
	HRESULT result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 키보드의 Direct Input 인터페이스를 생성합니다
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 데이터 형식을 설정하십시오. 이 경우 키보드이므로 사전 정의 된 데이터 형식을 사용할 수 있습니다.
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	// 다른 프로그램과 공유하지 않도록 키보드의 협조 수준을 설정합니다
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// 키보드를 할당받는다
	result = m_keyboard->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	// 마우스 Direct Input 인터페이스를 생성합니다.
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 미리 정의 된 마우스 데이터 형식을 사용하여 마우스의 데이터 형식을 설정합니다.
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	// 다른 프로그램과 공유 할 수 있도록 마우스의 협력 수준을 설정합니다.
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// 마우스를 할당받는다
	result = m_mouse->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void InputClass::Shutdown()
{
	// 마우스를 반환합니다.
	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}

	// 키보드를 반환합니다.
	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}

	// m_directInput 객체를 반환합니다.
	if (m_directInput)
	{
		m_directInput->Release();
		m_directInput = 0;
	}
}

bool InputClass::GetKeyDown(unsigned int input)
{
	// 키가 눌렸다면 해당 키값을 true로 저장합니다
	return  keyState[input] == KEY_DOWN;

}


bool InputClass::GetKeyUp(unsigned int input)
{
	// 키가 해제되었다면 해당 키값을 false로 저장합니다
	return keyState[input]== KEY_UP;
}


bool InputClass::GetKey(unsigned int key)
{
	// 현재 키값이 눌려졌는지 아닌지 상태를 반환합니다

	return keyState[key] == KEY_PRESSED;
	//return m_keyboardState[key] & 0x80;
}

bool InputClass::GetMouseDown(BYTE input)
{
	// 키가 눌렸다면 해당 키값을 true로 저장합니다
	return  mouseState[input] == KEY_DOWN;

}


bool InputClass::GetMouseUp(BYTE input)
{
	// 키가 해제되었다면 해당 키값을 false로 저장합니다
	return mouseState[input] == KEY_UP;
}


bool InputClass::GetMouse(BYTE key)
{
	// 현재 키값이 눌려졌는지 아닌지 상태를 반환합니다

	return mouseState[key] == KEY_PRESSED;
	//return m_keyboardState[key] & 0x80;
}



bool InputClass::GetMouseState()
{
	m_mouseState.rgbButtons[0];
	return false;
}
bool InputClass::ToggleMouseCursor(bool _toggle)
{
	toggleMouseCursor = _toggle;
	ShowCursor(_toggle);
	return _toggle;
}
bool InputClass::GetMouseCursorToggle()
{
	return toggleMouseCursor;
}
XMFLOAT3 InputClass::GetMouseAxis()
{
	if (toggleMouseCursor)
	{
		return XMFLOAT3(0, 0, 0);
	}
	return XMFLOAT3(m_mouseState.lX, m_mouseState.lY,0);
}
bool InputClass::Frame()
{
	// 키보드의 현재 상태를 읽는다.
	if (!ReadKeyboard())
	{
		//return false;
	}

	// 마우스의 현재 상태를 읽는다.
	if (!ReadMouse())
	{
		//return false;
	}

	// 키보드와 마우스의 변경상태를 처리합니다.
	ProcessInput();
	return true;
}

bool InputClass::ReadKeyboard()
{
	// 키보드 디바이스를 얻는다.
	HRESULT result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	if (FAILED(result))
	{
		// 키보드가 포커스를 잃었거나 획득되지 않은 경우 컨트롤을 다시 가져 온다
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_keyboard->Acquire();
		}
		else
		{
			return false;
		}
	}
	for (int i = 0; i < 256; i++)
	{
		if (keyState[i] == KEY_DOWN && m_keyboardState[i] & 0x80)
		{
			keyState[i] = KEY_PRESSED;
		}
		else if ((keyState[i] == KEY_PRESSED || keyState[i] == KEY_DOWN ) && !(m_keyboardState[i] & 0x80))
		{
			keyState[i] = KEY_UP;
		}
		else if (keyState[i] == KEY_UP && !(m_keyboardState[i] & 0x80))
		{
			keyState[i] = KEY_NONE;
		}
		else if(keyState[i] != KEY_PRESSED && (m_keyboardState[i] & 0x80))
		{
			keyState[i] = KEY_DOWN;
		}
	}
	return true;
}

bool InputClass::ReadMouse()
{
	// 마우스 디바이스를 얻는다.
	HRESULT result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result))
	{
		// 마우스가 포커스를 잃었거나 획득되지 않은 경우 컨트롤을 다시 가져 온다
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			return false;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if (mouseState[i] == KEY_DOWN && m_mouseState.rgbButtons[i] & 0x80)
		{
			mouseState[i] = KEY_PRESSED;
		}
		else if ((mouseState[i] == KEY_PRESSED || mouseState[i] == KEY_DOWN) && !(m_mouseState.rgbButtons[i] & 0x80))
		{
			mouseState[i] = KEY_UP;
		}
		else if (mouseState[i] == KEY_UP && !(m_mouseState.rgbButtons[i] & 0x80))
		{
			mouseState[i] = KEY_NONE;
		}
		else if (mouseState[i] != KEY_PRESSED && (m_mouseState.rgbButtons[i] & 0x80))
		{
			mouseState[i] = KEY_DOWN;
		}
	}
	return true;
}

void InputClass::ProcessInput()
{
	// 프레임 동안 마우스 위치의 변경을 기반으로 마우스 커서의 위치를 ​​업데이트 합니다.
	POINT pt;
	GetCursorPos(&pt);
	//m_mouseX += m_mouseState.lX;
	//m_mouseY += m_mouseState.lY;
	m_mouseX = pt.x;
	m_mouseY = pt.y;
	int screenSizeX = System()->GetScreenSize().x;
	int screenSizeY = System()->GetScreenSize().y;
	XMFLOAT2 windowPos = System()->GetWindowPos();

	// 마우스 위치가 화면 너비 또는 높이를 초과하지 않는지 확인한다.
	if (m_mouseX < 0) { m_mouseX = 0; }
	if (m_mouseY < 0) { m_mouseY = 0; }

	if (m_mouseX > screenSizeX) { m_mouseX = screenSizeX; }
	if (m_mouseY > screenSizeY) { m_mouseY = screenSizeY; }
	if (!toggleMouseCursor)
	{
		//m_screenWidth = (SM_CXSCREEN - screenWidth) / 2;
		//m_screenHeight = (SM_CYSCREEN - screenHeight) / 2;
		/*POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(SystemClass::GetInstance()->GetHWND(),&pt);
		SetCursorPos(pt.x, pt.y);*/
		SetCursorPos(GetSystemMetrics(SM_CXSCREEN)/2, GetSystemMetrics(SM_CYSCREEN)/2);
		//SetCursorPos((SM_CXSCREEN - m_screenWidth) / 2, (SM_CYSCREEN - m_screenHeight) / 2);
	}
}


bool InputClass::IsEscapePressed()
{
	// escape 키가 현재 눌려지고 있는지 bit값을 계산하여 확인한다.
	if (m_keyboardState[DIK_ESCAPE] & 0x80)
	{
		return true;
	}

	return false;
}


void InputClass::GetMouseLocation(int& mouseX, int& mouseY)
{
	XMFLOAT2 windowPos = System()->GetWindowPos();
	XMFLOAT2 windowSize = System()->GetWindowSize();
	if (m_mouseX <= windowPos.x)mouseX = 0;
	else if (m_mouseX > windowPos.x+windowSize.x)mouseX = windowSize.x;
	else mouseX = m_mouseX-windowPos.x;
	if (m_mouseY <= windowPos.y)mouseY = 0;
	else if (m_mouseY > windowPos.y + windowSize.y)mouseY = windowSize.y;
	else mouseY = m_mouseY - windowPos.y;
}
void InputClass::GetMouseLocationForScreen(int& mouseX, int& mouseY)
{
	XMFLOAT2 windowPos = System()->GetWindowPos();
	mouseX = m_mouseX;
	mouseY = m_mouseY;
}

