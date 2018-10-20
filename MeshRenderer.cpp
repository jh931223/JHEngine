#include "stdafx.h"
#include "MeshRenderer.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "MeshClass.h"
#include "MaterialClass.h"
MeshRenderer::MeshRenderer()
{
	SystemClass::GetInstance()->GetGraphics()->PushRenderer(this);
}


MeshRenderer::~MeshRenderer()
{
	SystemClass::GetInstance()->GetGraphics()->RemoveRenderer(this);
}


Mesh * MeshRenderer::GetMesh()
{
	return mesh;
}

void MeshRenderer::ReleaseMesh()
{
	if (mesh)
	{
		mesh->Shutdown();
		delete mesh;
	}
	mesh = 0;
}

void MeshRenderer::SetMesh(Mesh * _mesh)
{
	mesh = _mesh;
}

Material * MeshRenderer::GetMaterial()
{
	return material;
}

void MeshRenderer::SetMaterial(Material * _material)
{
	material = _material;
}

void MeshRenderer::Render(ID3D11DeviceContext * _deviceContext, XMMATRIX _view, XMMATRIX _proj,Material* customMaterial)
{	// ���� ������ ������ �������� �����մϴ�.
	if (!mesh)
		return;
	if (!material)
		return;
	XMFLOAT3 pos= transform()->GetWorldPosition()+boundary.centerOffset;
	if (Frustum::FrustumCheckCube(pos.x, pos.y, pos.z, boundary.size))
	{
		UINT stride = sizeof(VertexBuffer);
		UINT offset = 0;

		XMMATRIX _world = transform()->GetTransformMatrix();
		// ������ �� �� �ֵ��� �Է� ��������� ���� ���۸� Ȱ������ �����մϴ�.
		if (mesh->GetVertexBuffer())
			_deviceContext->IASetVertexBuffers(0, 1, (mesh->GetVertexBuffer()), &stride, &offset);
		// ������ �� �� �ֵ��� �Է� ��������� �ε��� ���۸� Ȱ������ �����մϴ�.
		if (mesh->GetIndexBuffer())
			_deviceContext->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		// ���� ���۷� �׸� �⺻���� �����մϴ�. ���⼭�� �ﰢ������ �����մϴ�.
		_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		if(customMaterial)
			customMaterial->GetShader()->Render(_deviceContext, mesh->GetIndexCount(), _world, _view, _proj, *customMaterial->GetParams());
		else material->GetShader()->Render(_deviceContext, mesh->GetIndexCount(), _world, _view, _proj, *material->GetParams());
#ifdef _DEBUG
		Frustum::frustumCulled++;
		Frustum::drawnVertex+=mesh->GetVertexCount();
#endif
	}
}

void MeshRenderer::Update()
{
}

void MeshRenderer::OnStart()
{
}

bool Frustum::FrustumCheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
	for (int i = 0; i<6; i++)
	{
		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter - radius), (yCenter - radius), (zCenter - radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter + radius), (yCenter - radius), (zCenter - radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter - radius), (yCenter + radius), (zCenter - radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter + radius), (yCenter + radius), (zCenter - radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter - radius), (yCenter - radius), (zCenter + radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter + radius), (yCenter - radius), (zCenter + radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter - radius), (yCenter + radius), (zCenter + radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter + radius), (yCenter + radius), (zCenter + radius),
			1.0f))) >= 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::FrustumCheckSphere(float xCenter, float yCenter, float zCenter, float radius)
{
	for (int i = 0; i<6; i++)
	{
		// ���� �ݰ��� �� frustum �ȿ� �ִ��� Ȯ���մϴ�.
		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet(xCenter, yCenter, zCenter, 1.0f))) < -radius)
			return false;
	}

	return true;
}


void Frustum::ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix)
{
	frustumCulled = 0;
	drawnVertex = 0;
	if (isLockFrustum)
		return;
	XMFLOAT4X4 pMatrix;
	XMStoreFloat4x4(&pMatrix, projectionMatrix);

	// ����ü���� �ּ� Z �Ÿ��� ����մϴ�.
	float zMinimum = -pMatrix._43 / pMatrix._33;
	float r = screenDepth / (screenDepth - zMinimum);

	// ������Ʈ �� ���� �ٽ� ���� ��Ŀ� �����մϴ�.
	pMatrix._33 = r;
	pMatrix._43 = -r * zMinimum;
	projectionMatrix = XMLoadFloat4x4(&pMatrix);

	// �� ��Ʈ������ ������Ʈ �� �������� ��Ʈ�������� ����ü ��Ʈ������ ����ϴ�.
	XMMATRIX finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

	// ���� ����� XMFLOAT4X4 �������� ��ȯ�մϴ�.
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, finalMatrix);

	// ����ü�� ����� ����� ����մϴ�.
	float x = (float)(matrix._14 + matrix._13);
	float y = (float)(matrix._24 + matrix._23);
	float z = (float)(matrix._34 + matrix._33);
	float w = (float)(matrix._44 + matrix._43);
	m_planes[0] = XMVectorSet(x, y, z, w);
	m_planes[0] = XMPlaneNormalize(m_planes[0]);

	// ����ü�� �� ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[1] = XMVectorSet(x, y, z, w);
	m_planes[1] = XMPlaneNormalize(m_planes[1]);

	// ����ü�� ���� ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[2] = XMVectorSet(x, y, z, w);
	m_planes[2] = XMPlaneNormalize(m_planes[2]);

	// ����ü�� ������ ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[3] = XMVectorSet(x, y, z, w);
	m_planes[3] = XMPlaneNormalize(m_planes[3]);

	// ����ü�� �� ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[4] = XMVectorSet(x, y, z, w);
	m_planes[4] = XMPlaneNormalize(m_planes[4]);

	// ����ü�� �Ʒ� ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[5] = XMVectorSet(x, y, z, w);
	m_planes[5] = XMPlaneNormalize(m_planes[5]);

	return;
}

XMVECTOR Frustum::m_planes[6];
bool Frustum::isLockFrustum=false;
int Frustum::frustumCulled = 0;
int Frustum::drawnVertex = 0;