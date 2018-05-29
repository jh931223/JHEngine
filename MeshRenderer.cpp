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
{	// ���� ������ ������ �������� �����մϴ�.
	UINT stride = sizeof(VertxBuffer);
	UINT offset = 0;

	// ������ �� �� �ֵ��� �Է� ��������� ���� ���۸� Ȱ������ �����մϴ�.
	_deviceContext->IASetVertexBuffers(0, 1, (mesh->GetVertexBuffer()), &stride, &offset);
	// ������ �� �� �ֵ��� �Է� ��������� �ε��� ���۸� Ȱ������ �����մϴ�.
	_deviceContext->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	// ���� ���۷� �׸� �⺻���� �����մϴ�. ���⼭�� �ﰢ������ �����մϴ�.
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	material->GetShader()->Render(_deviceContext, mesh->GetIndexCount(), _world, _view, _proj);
}

void MeshRenderer::Update()
{
}

void MeshRenderer::OnStart()
{
}
