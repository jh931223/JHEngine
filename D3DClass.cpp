#include "stdafx.h"
#include "d3dclass.h"


D3DClass::D3DClass()
{
}


D3DClass::D3DClass(const D3DClass& other)
{
}


D3DClass::~D3DClass()
{
}


bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{
	// ��������ȭ ���¸� �����մϴ�
	m_vsync_enabled = vsync;

	// DirectX �׷��� �������̽� ���丮�� �����մϴ�
	IDXGIFactory* factory = nullptr;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
	{
		return false;
	}

	// ���丮 ��ü�� ����Ͽ� ù��° �׷��� ī�� �������̽� ��͸� �����մϴ�
	IDXGIAdapter* adapter = nullptr;
	if (FAILED(factory->EnumAdapters(0, &adapter)))
	{
		return false;
	}

	// ���(�����)�� ���� ù��° ��͸� �����մϴ�.
	IDXGIOutput* adapterOutput = nullptr;
	if (FAILED(adapter->EnumOutputs(0, &adapterOutput)))
	{
		return false;
	}

	// ��� (�����)�� ���� DXGI_FORMAT_R8G8B8A8_UNORM ǥ�� ���Ŀ� �´� ��� ���� �����ɴϴ�
	unsigned int numModes = 0;
	if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL)))
	{
		return false;
	}

	// ������ ��� ����Ϳ� �׷���ī�� ������ ������ ����Ʈ�� �����մϴ�
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// ���� ���÷��� ��忡 ���� ����Ʈ�� ä��ϴ�
	if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList)))
	{
		return false;
	}

	// ���� ��� ���÷��� ��忡 ���� ȭ�� �ʺ�/���̿� �´� ���÷��� ��带 ã���ϴ�.
	// ������ ���� ã���� ������� ���ΰ�ħ ������ �и�� ���� ���� �����մϴ�.
	unsigned int numerator = 0;
	unsigned int denominator = 0;
	for (unsigned int i = 0; i<numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// ����ī���� ����ü�� ����ϴ�
	DXGI_ADAPTER_DESC adapterDesc;
	if (FAILED(adapter->GetDesc(&adapterDesc)))
	{
		return false;
	}

	// ����ī�� �޸� �뷮 ������ �ް�����Ʈ ������ �����մϴ�
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// ����ī���� �̸��� �����մϴ�
	size_t stringLength = 0;
	if (wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128) != 0)
	{
		return false;
	}

	// ���÷��� ��� ����Ʈ�� �����մϴ�
	delete[] displayModeList;
	displayModeList = 0;

	// ��� ��͸� �����մϴ�
	adapterOutput->Release();
	adapterOutput = 0;

	// ��͸� �����մϴ�
	adapter->Release();
	adapter = 0;

	// ���丮 ��ü�� �����մϴ�
	factory->Release();
	factory = 0;

	// ����ü�� ����ü�� �ʱ�ȭ�մϴ�
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// ����۸� 1���� ����ϵ��� �����մϴ�
	swapChainDesc.BufferCount = 1;

	// ������� ���̿� ���̸� �����մϴ�
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// 32bit �����̽��� �����մϴ�
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// ������� ���ΰ�ħ ������ �����մϴ�
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// ������� ���뵵�� �����մϴ�
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// �������� ���� ������ �ڵ��� �����մϴ�
	swapChainDesc.OutputWindow = hwnd;

	// ��Ƽ���ø��� ���ϴ�
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// â��� or Ǯ��ũ�� ��带 �����մϴ�
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// ��ĵ ���� ���� �� ũ�⸦ �������� �������� �����մϴ�.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// ��µ� ���� ����۸� ��쵵�� �����մϴ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// �߰� �ɼ� �÷��׸� ������� �ʽ��ϴ�
	swapChainDesc.Flags = 0;

	// ��ó������ DirectX 11 �� �����մϴ�
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// ���� ü��, Direct3D ��ġ �� Direct3D ��ġ ���ؽ�Ʈ�� ����ϴ�.
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_immDeviceContext)))
	{
		return false;
	}

	// ����� �����͸� ���ɴϴ�
	ID3D11Texture2D* backBufferPtr = nullptr;
	if (FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr)))
	{
		return false;
	}

	// �� ���� �����ͷ� ���� Ÿ�� �並 �����Ѵ�.
	if (FAILED(m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView)))
	{
		return false;
	}

	// ����� �����͸� �����մϴ�
	backBufferPtr->Release();
	backBufferPtr = 0;
	// ���� ���� ����ü�� �ʱ�ȭ�մϴ�
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// ���� ���� ����ü�� �ۼ��մϴ�
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// ������ ���̹��� ����ü�� ����Ͽ� ���� ���� �ؽ��ĸ� �����մϴ�
	if (FAILED(m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer)))
	{
		return false;
	}

	// ���ٽ� ���� ����ü�� �ʱ�ȭ�մϴ�
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// ���ٽ� ���� ����ü�� �ۼ��մϴ�
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// �ȼ� ������ ���ٽ� �����Դϴ�
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// �ȼ� �޸��� ���ٽ� �����Դϴ�
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// ���� ���ٽ� ���¸� �����մϴ�
	if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState)))
	{
		return false;
	}

	// ���� ���ٽ� ���¸� �����մϴ�
	m_immDeviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// ���� ���ٽ� ���� ����ü�� �ʱ�ȭ�մϴ�
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// ���� ���ٽ� �� ����ü�� �����մϴ�
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	

	// ���� ���ٽ� �並 �����մϴ�
	if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView)))
	{
		return false;
	}

	// ������ ��� ��� ���� ���ٽ� ���۸� ��� ���� ������ ���ο� ���ε��մϴ�
	m_immDeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	ChangeFillMode(true);



	// �������� ���� ����Ʈ�� �����մϴ�
	m_viewport.Width = (float)screenWidth;
	m_viewport.Height = (float)screenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	// ����Ʈ�� �����մϴ�
	m_immDeviceContext->RSSetViewports(1, &m_viewport);

	// ���� ����� �����մϴ�
	float fieldOfView = XM_PI / 4.0f;
	float screenAspect = (float)screenWidth / (float)screenHeight;

	// 3D ������������ ���� ����� ����ϴ�
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, 100000);

	// ���� ����� �׵� ��ķ� �ʱ�ȭ�մϴ�
	m_worldMatrix = XMMatrixIdentity();

	// 2D ������������ ���� ���� ����� ����ϴ�
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	// ���� 2D ������������ Z ���۸� ���� �� ��° ���� ���ٽ� ���¸� ����ϴ�. ������ ��������
	// DepthEnable�� false�� �����ϸ� �ٸ� ��� �Ű� ������ �ٸ� ���� ���ٽ� ���¿� �����մϴ�.
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// ��ġ�� ����Ͽ� ���¸� ����ϴ�.
	if (FAILED(m_device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState)))
	{
		return false;
	}

	// �׷����� ������� ����� ������ ������ ����ü�� �����մϴ�
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	//rasterDesc.CullMode = D3D11_CULL_NONE;
	//rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// ��� �ۼ��� ����ü���� ������ ������ ���¸� ����ϴ�
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, &m_cullOffState)))
	{
		return false;
	}
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	//rasterDesc.CullMode = D3D11_CULL_NONE;
	//rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// ��� �ۼ��� ����ü���� ������ ������ ���¸� ����ϴ�
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, &m_cullFrontState)))
	{
		return false;
	}

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendDesc, &m_blendState);

	D3D11_BLEND_DESC blendDesc2;
	ZeroMemory(&blendDesc2, sizeof(D3D11_BLEND_DESC));
	blendDesc2.RenderTarget[0].BlendEnable = FALSE;
	blendDesc2.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc2.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc2.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc2.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc2.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc2.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc2.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendDesc2, &m_blendDisableState);

	return true;
}


void D3DClass::Shutdown()
{
	// ���� �� ������ ���� �������� ������ ���� ü���� ���� �� �� ���ܰ� �߻��մϴ�.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}
	if (m_cullOffState)
	{
		m_cullOffState->Release();
		m_cullOffState = 0;
	}
	if (m_cullFrontState)
	{
		m_cullFrontState->Release();
		m_cullFrontState = 0;
	}
	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_immDeviceContext)
	{
		m_immDeviceContext->Release();
		m_immDeviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	if (m_blendState)
		m_blendState->Release();
	if (m_blendDisableState)
		m_blendDisableState->Release();
	ReleaseDeferredContex();
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	// ���۸� ���� ���� �����մϴ�
	float color[4] = { red, green, blue, alpha };

	// ����۸� ����ϴ�
	m_immDeviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// ���� ���۸� ����ϴ�
	m_immDeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


void D3DClass::EndScene()
{
	// �������� �Ϸ�Ǿ����Ƿ� ȭ�鿡 �� ���۸� ǥ���մϴ�.
	if (m_vsync_enabled)
	{
		// ȭ�� ���� ��ħ ������ �����մϴ�.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// ������ ������ ����մϴ�
		m_swapChain->Present(0, 0);
	}
}


ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* D3DClass::GetImmDeviceContext()
{
	return m_immDeviceContext;
}
ID3D11DeviceContext * D3DClass::GetDeferredContext(int index)
{
	if (deferredContexts.size() <= index)
		return NULL;
	return deferredContexts[index];
}
ID3D11CommandList * D3DClass::GetCommandList(int index)
{
	if (commandLists.size() <= index)
		return NULL;
	return commandLists[index];
}
int D3DClass::GetDeferredContextsSize()
{
	return deferredContexts.size();
}
void D3DClass::CreateDeferredContext(int num)
{
	ID3D11DeviceContext* DDC=NULL;
	maxDeferredContextNum = num;
	for (int i = 0; i < maxDeferredContextNum; i++)
	{
		m_device->CreateDeferredContext(i, &DDC);
		deferredContexts.push_back(DDC);
	}
	commandLists.resize(num);
}

bool D3DClass::ReleaseDeferredContex()
{
	for (int i = 0; i<maxDeferredContextNum; i++)
	{
		if (deferredContexts[i])
		{
			deferredContexts[i]->Release();
			delete deferredContexts[i];
		}
		if (commandLists[i])
		{
			commandLists[i]->Release();
		}
	}
	deferredContexts.clear();
	commandLists.clear();
	return true;
}


void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
}


void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
}


void D3DClass::SetBackBufferRenderTarget()
{
	// ���� Ÿ�� ��� ���� ���ٽ� ���۸� ��� ���� ������ ���ο� ���ε��մϴ�.
	m_immDeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	ResetViewport();
}


void D3DClass::ResetViewport()
{
	// ����Ʈ�� �缳���մϴ�.
	m_immDeviceContext->RSSetViewports(1, &m_viewport);
}

void D3DClass::ChangeFillMode(bool isSolid)
{

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	// �׷����� ������� ����� ������ ������ ����ü�� �����մϴ�
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	isRenderWireFrame = !isSolid;
	if (!isRenderWireFrame)
	{
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
	}
	else
	{
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	}
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// ��� �ۼ��� ����ü���� ������ ������ ���¸� ����ϴ�
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, &m_rasterState)))
	{
		return;
	}

	// ���� ������ ������ ���¸� �����մϴ�
	m_immDeviceContext->RSSetState(m_rasterState);

	
}

void D3DClass::TurnZBufferOn()
{
	m_immDeviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
}


void D3DClass::TurnZBufferOff()
{
	m_immDeviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
}

void D3DClass::TurnCullOff()
{
	m_immDeviceContext->RSSetState(m_cullOffState);
}

void D3DClass::TurnCullBack()
{
	m_immDeviceContext->RSSetState(m_rasterState);
}

void D3DClass::TurnCullFront()
{
	m_immDeviceContext->RSSetState(m_cullFrontState);
}

void D3DClass::TurnOnAlphaBlending()
{

	FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	m_immDeviceContext->OMSetBlendState(m_blendState, blendFactor, 0xffffffff);
}

void D3DClass::TurnOffAlphaBlending()
{

	FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	m_immDeviceContext->OMSetBlendState(m_blendDisableState, blendFactor, 0xffffffff);
}