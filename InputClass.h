#pragma once
#include"Singleton.h"

class InputClass : public Singleton<InputClass>
{
public:
	InputClass();
	~InputClass();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool GetKey(unsigned int);
	bool GetKeyDown(unsigned int);
	void MouseUpdate();
	void ToggleCursor();
	XMFLOAT3 GetCursorAxis();
	XMFLOAT3 GetCursorPosition();
private:
	bool  useCursor;
	XMFLOAT3 centerOfScreen;
	bool m_keys[256];
	bool m_keysPressing[256];
	XMFLOAT3 lastCursorPosition;
};

inline InputClass* Input() { return InputClass::GetInstance(); }