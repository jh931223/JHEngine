#pragma once
#include "ShaderClass.h"
class DepthMapShaderClass :
	public ShaderClass
{
public:
	DepthMapShaderClass();
	~DepthMapShaderClass();

	// ShaderClass을(를) 통해 상속됨
	virtual bool Initialize(ID3D11Device * device, HWND hwnd) override;
	virtual bool BuildShader(ID3D11Device * device, HWND hwnd, const WCHAR * vsFilename, const WCHAR * psFilename, const WCHAR * gsFileName = NULL) override;
	virtual void ShutdownShaderCustomBuffer() override;
	virtual bool DrawCall(ID3D11DeviceContext * deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, PARAM & params) override;
	virtual void RenderShader(ID3D11DeviceContext * deviceContext, int indexCount) override;
private:
	ID3D11Buffer * lightMatrixBuffer;

	struct LightMatrixBufferType
	{
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};
};

