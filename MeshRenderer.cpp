#include "stdafx.h"
#include "MeshRenderer.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "MeshClass.h"
#include "MaterialClass.h"
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

void MeshRenderer::Render(ID3D11DeviceContext * _deviceContext, XMMATRIX _world, XMMATRIX _view, XMMATRIX _proj)
{	// 정점 버퍼의 단위와 오프셋을 설정합니다.
	UINT stride = sizeof(VertxBuffer);
	UINT offset = 0;

	// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
	_deviceContext->IASetVertexBuffers(0, 1, (mesh->GetVertexBuffer()), &stride, &offset);
	// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
	_deviceContext->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	material->GetShader()->Render(_deviceContext, mesh->GetIndexCount(), _world, _view, _proj);
}

void MeshRenderer::Update()
{
}

void MeshRenderer::OnStart()
{
}
