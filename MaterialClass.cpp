#include "stdafx.h"
#include "ShaderClass.h"
#include "TextureClass.h"
#include "MaterialClass.h"
#include "SystemClass.h"
#include <vector>

Material::Material()
{
}

Material::Material(const Material & mat)
{
	this->params = mat.params;
	this->shader = mat.shader;
}


Material::~Material()
{
	//if (shader)
	//{
	//	shader->Shutdown();
	//	shader = 0;
	//}
}

bool Material::Render(ID3D11DeviceContext * _deviceContext, XMMATRIX _world, XMMATRIX _view, XMMATRIX _proj)
{
	return shader->Render(_deviceContext, _world, _view, _proj,params);
}

void Material::SetShader(ShaderClass * _shader)
{
	shader = _shader;
	//shader->Initialize(SystemClass::GetInstance()->GetDevice(), hwnd);
}

ShaderParameterCollections * Material::GetParams()
{
	return &params;
}
