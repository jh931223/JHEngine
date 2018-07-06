#pragma once
#include "ShaderClass.h"
class DepthShaderClass :
	public ShaderClass
{
public:
	DepthShaderClass();
	DepthShaderClass(const DepthShaderClass&);
	~DepthShaderClass();

	bool Initialize(ID3D11Device*, HWND)override;

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFileName = NULL)override;
	void ShutdownShaderCustomBuffer();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const WCHAR*);

	bool DrawCall(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, PARAM& params)override;
	void RenderShader(ID3D11DeviceContext*, int);

};

