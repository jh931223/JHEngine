#pragma once
#include"Singleton.h"

class InputClass : public Singleton<InputClass>
{
public:
	InputClass();
	~InputClass();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

	bool IsEscapePressed();
	void GetMouseLocation(int&, int&);


	bool GetKeyDown(unsigned int);
	bool GetKeyUp(unsigned int);
	bool GetKey(unsigned int);

	bool GetMouseState();

	bool ToggleMouseCursor(bool _toggle);

	bool GetMouseCursorToggle();

	XMFLOAT3 GetMouseAxis();
private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();
private:

	IDirectInput8* m_directInput = nullptr;
	IDirectInputDevice8* m_keyboard = nullptr;
	IDirectInputDevice8* m_mouse = nullptr;

	enum  KEY_STATE
	{
		KEY_NONE,
		KEY_DOWN,
		KEY_PRESSED,
		KEY_UP
	};

	unsigned char m_keyboardState[256] = { 0, };
	unsigned char keyState[256] = { KEY_NONE, };
	DIMOUSESTATE m_mouseState;

	bool toggleMouseCursor=true;

	int m_screenWidth = 0;
	int m_screenHeight = 0;

	int m_mouseX = 0;
	int m_mouseY = 0;
};

inline InputClass* Input() { return InputClass::GetInstance(); }