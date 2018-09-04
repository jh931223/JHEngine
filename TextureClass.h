#pragma once

class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	TextureClass(ID3D11Device*, const WCHAR*, bool isWIC = true);
	~TextureClass();

	bool Initialize(ID3D11Device*, const WCHAR*);
	bool InitializeWIC(ID3D11Device*, const WCHAR*);
	void Shutdown();

	ID3D11ShaderResourceView* const* GetResourceView();


private:
	ID3D11ShaderResourceView* m_texture = nullptr;
};