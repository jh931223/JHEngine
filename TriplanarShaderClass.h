#pragma once
#include "ShaderClass.h"
class TriplanarShaderClass :
	public ShaderClass
{
public:
	TriplanarShaderClass();
	~TriplanarShaderClass();


	// ShaderClass을(를) 통해 상속됨
	virtual bool Initialize(ID3D11Device * device, HWND hwnd) override;

	virtual bool InitializeShader(ID3D11Device * device, HWND hwnd, const WCHAR * vsFilename, const WCHAR * psFilename, const WCHAR * gsFileName = NULL) override;

	virtual void ShutdownShaderCustomBuffer() override;

	virtual bool DrawCall(ID3D11DeviceContext * deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, PARAM & params) override;

	virtual void RenderShader(ID3D11DeviceContext * deviceContext, int indexCount) override;

	// ShaderClass을(를) 통해 상속됨
};

