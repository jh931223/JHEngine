#pragma once
#include"Singleton.h"
class InputClass;
class GraphicsClass;
class HierachyClass;
class ResourcesClass;
class SystemClass : public Singleton<SystemClass>
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
	GraphicsClass* GetGraphics();
	ResourcesClass* GetResources();
	ID3D11Device* GetDevice();

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();
private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input = nullptr;
	GraphicsClass* m_Graphics = nullptr;
	HierachyClass* m_Hierachy = nullptr;
	ResourcesClass* m_Resources = nullptr;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;