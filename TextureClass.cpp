#include "stdafx.h"
#include "TextureClass.h"


TextureClass::TextureClass()
{
}


TextureClass::TextureClass(const TextureClass& other)
{
}

TextureClass::TextureClass(ID3D11Device* device, const WCHAR* filename)
{
	Initialize(device, filename);
}


TextureClass::~TextureClass()
{
}



bool TextureClass::Initialize(ID3D11Device* device, const WCHAR* filename)
{
	// 텍스처를 파일로부터 읽어온다
	if(FAILED(CreateDDSTextureFromFile(device, filename, nullptr, &m_texture)))
	{
		return false;
	}
	return true;
}


void TextureClass::Shutdown()
{
	//텍스처 뷰 리소스를 해제한다.
	if (m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}
}


ID3D11ShaderResourceView* const *TextureClass::GetResourceView()
{
	return &m_texture;
}

