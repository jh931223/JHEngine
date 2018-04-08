#include "stdafx.h"
#include "ShaderClass.h"
#include "TextureClass.h"
#include "MaterialClass.h"

MaterialClass::MaterialClass()
{
}


MaterialClass::~MaterialClass()
{
}

bool MaterialClass::Render(ID3D11DeviceContext * _deviceContext, const int _index, XMMATRIX _world, XMMATRIX _view, XMMATRIX _proj)
{
	return shader->Render(_deviceContext, _index, _world, _view, _proj, GetTextureArray());
}

void MaterialClass::SetTextureArray(int _index, TextureClass * _texture)
{
	if (textureArray.size() < _index)
		textureArray[_index] = _texture;
}

void MaterialClass::AddTexture(TextureClass * _texture)
{
	textureArray.push_back(_texture);
}

 std::vector<TextureClass*>* MaterialClass::GetTextureArray()
{
	return &textureArray;
}

void MaterialClass::SetShader(ShaderClass * _shader)
{
	shader = _shader;
}
