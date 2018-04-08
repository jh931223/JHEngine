#include "stdafx.h"
#include "MeshRenderer.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
MeshRenderer::MeshRenderer()
{
	SystemClass::GetInstance()->GetGraphics()->meshRenderers.push_back(this);
}


MeshRenderer::~MeshRenderer()
{
	SystemClass::GetInstance()->GetGraphics()->meshRenderers.remove(this);
}

TextureClass * MeshRenderer::GetTexture()
{
	return texture;
}

Mesh * MeshRenderer::GetMesh()
{
	return mesh;
}

void MeshRenderer::SetMesh(Mesh * _mesh)
{
	mesh = _mesh;
}

MaterialClass * MeshRenderer::GetMaterial()
{
	return material;
}

void MeshRenderer::SetMaterial(MaterialClass * _material)
{
	material = _material;
}

void MeshRenderer::Update()
{
}

void MeshRenderer::OnStart()
{
}
