#pragma once
#include<vector>



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
	ID3D11DeviceContext* GetDeferredContext(int index);
	ID3D11CommandList* GetCommandList(int index);
	int GetDeferredContextsSize();
	void CreateDeferredContext(int num);

	bool ReleaseDeferredContex();


	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

	void SetBackBufferRenderTarget();
	void ResetViewport();

	void ChangeFillMode(bool isSolid=true);

	void TurnZBufferOn();
	void TurnZBufferOff();
	void TurnCullOff();
	void TurnCullBack();

	void TurnCullFront();

	void TurnOnAlphaBlending();

	void TurnOffAlphaBlending();


	std::vector<ID3D11CommandList*> commandLists;
	std::vector<ID3D11DeviceContext*> deferredContexts;
private:
	bool m_vsync_enabled = false;
	int m_videoCardMemory = 0;
	char m_videoCardDescription[128] = { 0, };
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_immDeviceContext = nullptr;
	int maxDeferredContextNum = 0;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11Texture2D* m_depthStencilBuffer = nullptr;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11RasterizerState* m_rasterState = nullptr;

	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_orthoMatrix;
	D3D11_VIEWPORT m_viewport;

	ID3D11DepthStencilState* m_depthDisabledStencilState = NULL;
	ID3D11RasterizerState* m_cullOffState = nullptr;
	ID3D11RasterizerState* m_cullFrontState = nullptr;

	ID3D11BlendState* m_blendState = NULL;
	ID3D11BlendState* m_blendDisableState = NULL;
	bool isRenderWireFrame=false;
};