#include "stdafx.h"
#include "ShaderClass.h"
#include "TextureClass.h"
#include "MaterialClass.h"
#include "SystemClass.h"
#include <vector>

MaterialClass::MaterialClass()
{
}


MaterialClass::~MaterialClass()
{
	if (shader)
	{
		shader->Shutdown();
		shader = 0;
	}
}

bool MaterialClass::Render(ID3D11DeviceContext * _deviceContext, const int _index, XMMATRIX _world, XMMATRIX _view, XMMATRIX _proj)
{
	return shader->Render(_deviceContext, _index, _world, _view, _proj);
}

void MaterialClass::SetShader(ShaderClass * _shader,HWND hwnd)
{
	shader = _shader;
	shader->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
}

ShaderParameterCollections * MaterialClass::GetParams()
{
	return shader->GetParameters();
}
