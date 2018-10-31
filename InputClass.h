#pragma once
#include"Singleton.h"
#define KEY_LBUTTON 0x00
#define KEY_RBUTTON 0x01
#define KEY_CANCLE 0x02
#define KEY_MBUTTON 0x03
class InputClass : public Singleton<InputClass>
{
public:
	InputClass();
	~InputClass();

	bool Initialize(HINSTANCE, HWND);
	void Shutdown();
	bool Frame();

	bool IsEscapePressed();
	void GetMouseLocation(int&, int&);

	void GetMouseLocationForScreen(int & mouseX, int & mouseY);


	bool GetKeyDown(unsigned int);
	bool GetKeyUp(unsigned int);
	bool GetKey(unsigned int);

	bool GetMouseDown(BYTE input);

	bool GetMouseUp(BYTE input);

	bool GetMouse(BYTE key);

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
	unsigned char mouseState[4] = { KEY_NONE, };

	bool toggleMouseCursor=true;

	int m_windowWidth = 0;
	int m_windowHeight = 0;

	int m_mouseX = 0;
	int m_mouseY = 0;
};

inline InputClass* Input() { return InputClass::GetInstance(); }