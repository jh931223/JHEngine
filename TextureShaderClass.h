#pragma once
#include "ShaderClass.h"
class TextureShaderClass : public ShaderClass
{
public:
	TextureShaderClass();
	TextureShaderClass(const TextureShaderClass&);
	~TextureShaderClass();

	bool Initialize(ID3D11Device*, HWND) override;
	void Shutdown()override;
	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, std::vector<TextureClass*>* _textureArray)override;

private:
	bool InitializeShader(ID3D11Device*, HWND, const WCHAR*, const WCHAR*)override;
	void ShutdownShader()override;

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, std::vector<TextureClass*>* _textureArray)override;
	void RenderShader(ID3D11DeviceContext*, int)override;
};
