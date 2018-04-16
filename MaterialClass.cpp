#include "stdafx.h"
#include "ShaderClass.h"
#include "TextureClass.h"
#include "MaterialClass.h"
#include <vector>

MaterialClass::MaterialClass()
{
}


MaterialClass::~MaterialClass()
{
}

bool MaterialClass::Render(ID3D11DeviceContext * _deviceContext, const int _index, XMMATRIX _world, XMMATRIX _view, XMMATRIX _proj)
{
	return shader->Render(_deviceContext, _index, _world, _view, _proj, &params);
}

void MaterialClass::SetShader(ShaderClass * _shader)
{
	shader = _shader;
}

ShaderParameterCollections * MaterialClass::GetParams()
{
	return &params;
}
