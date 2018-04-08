#pragma once

class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	TextureClass(ID3D11Device*, const WCHAR*);
	~TextureClass();

	bool Initialize(ID3D11Device*, const WCHAR*);
	void Shutdown();

	ID3D11ShaderResourceView* const* GetTexture();


private:
	ID3D11ShaderResourceView* m_texture = nullptr;
};