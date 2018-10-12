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
	int GetScreenWidth();
	int GetScreenHeight();
	GraphicsClass* GetGraphics();
	D3DClass* GetD3D();
	ResourcesClass* GetResources();
	ID3D11Device* GetDevice();
	std::vector<SceneClass*>* GetSceneList();
	SceneClass* GetMainScene();
	HWND& GetHWND();
	int DeltaTime() { return deltaTime; }
private:
	clock_t deltaTimeCheck = 0;
	int deltaTime = 0;
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();
private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input = nullptr;
	GraphicsClass* m_Graphics = nullptr;
	std::vector<SceneClass*> sceneList;
	SceneClass* mainScene;
	ResourcesClass* m_Resources = nullptr;
	int screenWidth = 0;
	int screenHeight = 0;
	bool printFrame = true;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;