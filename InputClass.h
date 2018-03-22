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

	bool IsKeyDown(unsigned int);
	void MouseUpdate();
	XMFLOAT3 GetCursorAxis();
	XMFLOAT3 GetCursorPosition();
private:

	bool m_keys[256];
	XMFLOAT3 lastCursorPosition;
};

inline InputClass* Input() { return InputClass::GetInstance(); }