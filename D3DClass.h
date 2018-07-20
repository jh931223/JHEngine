#pragma once
#include<vector>


struct DEFERRED_CONTEXT_BUFFER
{
	ID3D11DeviceContext* context;
	ID3D11CommandList* commandList;
	void FinishCommandList();
	void ExcuteCommandList(ID3D11DeviceContext* immContext);
};

class D3DClass : public AlignedAllocationPolicy<16>
{
public:
	D3DClass();
	D3DClass(const D3DClass&);
	virtual ~D3DClass();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetImmDeviceContext();
	DEFERRED_CONTEXT_BUFFER* CreateDeferredContext();

	bool ReleaseDeferredContex(DEFERRED_CONTEXT_BUFFER * _context);

	bool ExcuteCommandLists();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

	void SetBackBufferRenderTarget();
	void ResetViewport();


private:
	bool m_vsync_enabled = false;
	int m_videoCardMemory = 0;
	char m_videoCardDescription[128] = { 0, };
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_immDeviceContext = nullptr;
	std::vector<DEFERRED_CONTEXT_BUFFER> m_deferredContexts;
	const int maxDeferredContextNum = 8;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11Texture2D* m_depthStencilBuffer = nullptr;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11RasterizerState* m_rasterState = nullptr;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_orthoMatrix;
	D3D11_VIEWPORT m_viewport;
};