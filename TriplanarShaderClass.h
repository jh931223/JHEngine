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

	virtual bool BuildShader(ID3D11Device * device, HWND hwnd, const WCHAR * vsFilename, const WCHAR * psFilename, const WCHAR * gsFileName = NULL) override;

	virtual void ShutdownShaderCustomBuffer() override;

	virtual bool DrawCall(ID3D11DeviceContext * deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, PARAM & params) override;

	virtual void RenderShader(ID3D11DeviceContext * deviceContext) override;

	// ShaderClass을(를) 통해 상속됨

	struct PSLightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		float shadowBias;
		XMFLOAT3 padding;
	};
	struct VSLightBufferType
	{
		XMFLOAT4 cameraPos;
		XMFLOAT3 lightDir;
		float padding;
	};

	struct LightMatrixBufferType
	{
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};

	ID3D11Buffer* psLightBuffer;
	ID3D11Buffer* vsLightBuffer;
	ID3D11Buffer * lightMatrixBuffer;

	std::map<ID3D11DeviceContext*,ID3D11Buffer*> psLightBufferPerDC;
	std::map<ID3D11DeviceContext*, ID3D11Buffer*> vsLightBufferPerDC;
};

