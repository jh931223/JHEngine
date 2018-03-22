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

void MeshRenderer::Update()
{
}

void MeshRenderer::OnStart()
{
}
