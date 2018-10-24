#include "stdafx.h"
#include "BitmapRenderer.h"
#include "BitmapClass.h"
#include "MaterialClass.h"
#include "GraphicsClass.h"
#include "D3DClass.h"
#include "SystemClass.h"

BitmapRenderer::BitmapRenderer()
{
	SystemClass::GetInstance()->GetGraphics()->PushBitmapRenderer(this);
}


BitmapRenderer::~BitmapRenderer()
{
	SystemClass::GetInstance()->GetGraphics()->RemoveBitmapRenderer(this);
	if (bitmap)
	{
		bitmap->Shutdown();
		delete bitmap;
		bitmap = 0;
	}
}

Material * BitmapRenderer::GetMaterial()
{
	return material;
}

void BitmapRenderer::SetMaterial(Material * _material)
{
	material = _material;
}

void BitmapRenderer::SetBitmap(int _width, int _height)
{
	if (bitmap)
	{
		bitmap->Shutdown();
		delete bitmap;
	}
	bitmap = new BitmapClass;
	auto system = SystemClass::GetInstance();
	bitmap->Initialize(system->GetDevice(), system->GetScreenWidth(), system->GetScreenHeight(), _width, _height);
}

void BitmapRenderer::Render(ID3D11DeviceContext * _deviceContext, XMMATRIX _view, XMMATRIX _proj)
{	// 정점 버퍼의 단위와 오프셋을 설정합니다.
	if (!bitmap)
		return;
	if (!material)
		return;
	UINT stride = sizeof(BitmapClass::VertexType);
	UINT offset = 0;

	//XMMATRIX _world = transform()->GetTransformMatrix();
	//XMMATRIX _world=CameraComponent::mainCamera()->transform()->GetTransformMatrix();
	XMMATRIX _world;
	SystemClass::GetInstance()->GetD3D()->GetWorldMatrix(_world);
	bitmap->Render(_deviceContext, transform()->GetWorldPosition().x, transform()->GetWorldPosition().y);
	material->GetShader()->Render(_deviceContext, _world, _view, _proj, *material->GetParams());
	_deviceContext->DrawIndexed(bitmap->GetIndexCount(), 0, 0);
}