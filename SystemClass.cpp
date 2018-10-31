#include "stdafx.h"
#include "inputclass.h"
#include "graphicsclass.h"
#include "systemclass.h"
#include "SceneClass.h"
#include"resourcesclass.h"
#include "D3DClass.h"
#include<time.h>
#include"Timer.h"
//��
#include "MainScene.h"
//
SystemClass::SystemClass()
{
}


SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	// ������ â ����, ���� ���� ���� �ʱ�ȭ

	// ������ ���� �ʱ�ȭ
	InitializeWindows(windowSize.x, windowSize.y);


	// m_Input ��ü ����. �� Ŭ������ ���� ������� Ű���� �Է� ó���� ���˴ϴ�.
	m_Input = InputClass::GetInstance();
	if (!m_Input)
	{
		return false;
	}

	// m_Input ��ü �ʱ�ȭ
	m_Input->Initialize(m_hinstance, m_hwnd);

	// m_Graphics ��ü ����.  �׷��� �������� ó���ϱ� ���� ��ü�Դϴ�.
	m_Graphics = GraphicsClass::GetInstance();
	if (!m_Graphics)
	{
		return false;
	}
	// m_Graphics ��ü �ʱ�ȭ.
	m_Graphics->Initialize(windowSize.x, windowSize.y, m_hwnd);

	m_Resources = ResourcesClass::GetInstance();
	if (!m_Resources)
		return false;
	m_Resources->Initialize(m_hwnd);


	sceneList.push_back(new MainScene());

	for (auto i : sceneList)
		i->Setup();
	for (auto i : sceneList)
		i->Start();

	return true;
}


void SystemClass::Shutdown()
{

	if (sceneList.size())
	{
		for (auto i : sceneList)
			delete i;
		sceneList.clear();
	}
	// m_Graphics ��ü ��ȯ
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// m_Input ��ü ��ȯ
	if (m_Input)
	{
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	

	// Window ���� ó��
	ShutdownWindows();
}


void SystemClass::Run()
{
	// �޽��� ����ü ���� �� �ʱ�ȭ
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// ����ڷκ��� ���� �޽����� ���������� �޽��������� ���ϴ�
	while (true)
	{
		// ������ �޽����� ó���մϴ�
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// ���� �޽����� ���� ��� �޽��� ������ Ż���մϴ�
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// �� �ܿ��� Frame �Լ��� ó���մϴ�.
			if (!Frame())
				break;
		}
		if (m_Input->IsEscapePressed())
		{
			break;
		}
	}
}


bool SystemClass::Frame()
{
	Timer::TimerUpdate();
	m_Input->Frame();
	for(auto i:sceneList)
		i->Update();
	// �׷��� ��ü�� Frame�� ó���մϴ�
	m_Graphics->Frame();
	if(printFrame)
		printf("Frame : %f fps\n",1.0f / Timer::DeltaTime());
	return true;
}



LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		// Ű���尡 �������°� ó��

	// �� ���� ��� �޽������� �⺻ �޽��� ó���� �ѱ�ϴ�.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}


int SystemClass::GetWindowWidth()
{
	return m_Graphics->GetWindowSize().x;
}
int SystemClass::GetWindowHeight()
{
	return m_Graphics->GetWindowSize().y;
}

XMFLOAT2 SystemClass::GetWindowPos()
{
	return windowPos;
}

XMFLOAT2 SystemClass::GetWindowSize()
{
	return  m_Graphics->GetWindowSize();
}

XMFLOAT2 SystemClass::GetScreenSize()
{
	return XMFLOAT2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

void SystemClass::InitializeWindows(int& _screenWidth, int& _screenHeight)
{
	// �ܺ� �����͸� �� ��ü�� �����մϴ�
	ApplicationHandle = this;

	// �� ���α׷��� �ν��Ͻ��� �����ɴϴ�
	m_hinstance = GetModuleHandle(NULL);

	// ���α׷� �̸��� �����մϴ�
	m_applicationName = L"JHEngine";

	// windows Ŭ������ �Ʒ��� ���� �����մϴ�.
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// windows class�� ����մϴ�
	RegisterClassEx(&wc);

	// ����� ȭ���� �ػ󵵸� �о�ɴϴ�
	_screenWidth = GetSystemMetrics(SM_CXSCREEN);
	_screenHeight = GetSystemMetrics(SM_CYSCREEN);


	// FULL_SCREEN ���� ���� ���� ȭ���� �����մϴ�.
	if (FULL_SCREEN)
	{
		// Ǯ��ũ�� ���� �����ߴٸ� ����� ȭ�� �ػ󵵸� ����ũ�� �ػ󵵷� �����ϰ� ������ 32bit�� �����մϴ�.
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)_screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)_screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Ǯ��ũ������ ���÷��� ������ �����մϴ�.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		windowPos.x = (GetSystemMetrics(SM_CXSCREEN));
		windowPos.y = (GetSystemMetrics(SM_CYSCREEN));
	}
	else
	{
		// ������ ����� ��� 800 * 600 ũ�⸦ �����մϴ�.
		_screenWidth= 800;
		_screenHeight = 600;

		// ������ â�� ����, ������ �� ��� ������ �մϴ�.
		windowPos.x = (GetSystemMetrics(SM_CXSCREEN) - _screenWidth) / 2;
		windowPos.y = (GetSystemMetrics(SM_CYSCREEN) - _screenHeight) / 2;
	}

	// �����츦 �����ϰ� �ڵ��� ���մϴ�.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		windowPos.x, windowPos.y, _screenWidth, _screenHeight, NULL, NULL, m_hinstance, NULL);

	// �����츦 ȭ�鿡 ǥ���ϰ� ��Ŀ���� �����մϴ�
	SetCursor(NULL);
	ShowCursor(false);
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);
}


void SystemClass::ShutdownWindows()
{
	// Ǯ��ũ�� ��忴�ٸ� ���÷��� ������ �ʱ�ȭ�մϴ�.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// â�� �����մϴ�
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// ���α׷� �ν��Ͻ��� �����մϴ�
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// �ܺ������� ������ �ʱ�ȭ�մϴ�
	ApplicationHandle = NULL;
}

GraphicsClass * SystemClass::GetGraphics()
{
	return m_Graphics;
}

D3DClass * SystemClass::GetD3D()
{
	return m_Graphics->GetD3D();
}

ResourcesClass * SystemClass::GetResources()
{
	return m_Resources;
}

ID3D11Device * SystemClass::GetDevice()
{
	if (m_Graphics)
	{
		return m_Graphics->GetD3D()->GetDevice();
	}
	return nullptr;
}

std::vector<SceneClass*> * SystemClass::GetSceneList()
{
	return &sceneList;
}

SceneClass * SystemClass::GetMainScene()
{
	if (mainScene == NULL)
		mainScene = sceneList[0];
	return mainScene;
}

HWND & SystemClass::GetHWND()
{
	return m_hwnd;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// ������ ���Ḧ Ȯ���մϴ�
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// �����찡 �������� Ȯ���մϴ�
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}


	// �� ���� ��� �޽������� �ý��� Ŭ������ �޽��� ó���� �ѱ�ϴ�.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}