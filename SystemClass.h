#pragma once
#include"Singleton.h"
#include<vector>
#include<time.h>
class InputClass;
class GraphicsClass;
class SceneClass;
class ResourcesClass;
class D3DClass;
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
	int GetWindowWidth();
	int GetWindowHeight();
	XMFLOAT2 GetWindowPos();
	XMFLOAT2 GetWindowSize();
	XMFLOAT2 GetScreenSize();
	GraphicsClass* GetGraphics();
	D3DClass* GetD3D();
	ResourcesClass* GetResources();
	ID3D11Device* GetDevice();
	std::vector<SceneClass*>* GetSceneList();
	SceneClass* GetMainScene();
	HWND& GetHWND();
private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();
private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	XMFLOAT2 windowPos;
	XMINT2 windowSize;

	InputClass* m_Input = nullptr;
	GraphicsClass* m_Graphics = nullptr;
	std::vector<SceneClass*> sceneList;
	SceneClass* mainScene;
	ResourcesClass* m_Resources = nullptr;
	bool printFrame = false;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;
inline SystemClass* System() { return SystemClass::GetInstance(); }