#include "stdafx.h"
#include "TextureClass.h"


TextureClass::TextureClass()
{
}


TextureClass::TextureClass(const TextureClass& other)
{
}

TextureClass::TextureClass(ID3D11Device* device, const WCHAR* filename, bool isWIC)
{
	if(!isWIC)
		Initialize(device, filename);
	else
		InitializeWIC(device, filename);
}


TextureClass::~TextureClass()
{
}



bool TextureClass::Initialize(ID3D11Device* device, const WCHAR* filename)
{
	// �ؽ�ó�� ���Ϸκ��� �о�´�
	if(FAILED(CreateDDSTextureFromFile(device, filename, nullptr, &m_texture)))
	{
		return false;
	}
	//if (FAILED(CreateWICTextureFromFile(device, filename, nullptr, &m_texture)))
	//{
	//	return false;
	//}
	return true;
}

bool TextureClass::InitializeWIC(ID3D11Device* device, const WCHAR* filename)
{
	// �ؽ�ó�� ���Ϸκ��� �о�´�
	if (FAILED(CreateWICTextureFromFile(device, filename, nullptr, &m_texture)))
	{
		return false;
	}
	return true;
}


void TextureClass::Shutdown()
{
	//�ؽ�ó �� ���ҽ��� �����Ѵ�.
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

