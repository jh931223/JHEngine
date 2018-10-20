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
{	// 정점 버퍼의 단위와 오프셋을 설정합니다.
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
		// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
		if (mesh->GetVertexBuffer())
			_deviceContext->IASetVertexBuffers(0, 1, (mesh->GetVertexBuffer()), &stride, &offset);
		// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
		if (mesh->GetIndexBuffer())
			_deviceContext->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
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
		// 구의 반경이 뷰 frustum 안에 있는지 확인합니다.
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

	// 절두체에서 최소 Z 거리를 계산합니다.
	float zMinimum = -pMatrix._43 / pMatrix._33;
	float r = screenDepth / (screenDepth - zMinimum);

	// 업데이트 된 값을 다시 투영 행렬에 설정합니다.
	pMatrix._33 = r;
	pMatrix._43 = -r * zMinimum;
	projectionMatrix = XMLoadFloat4x4(&pMatrix);

	// 뷰 매트릭스와 업데이트 된 프로젝션 매트릭스에서 절두체 매트릭스를 만듭니다.
	XMMATRIX finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

	// 최종 행렬을 XMFLOAT4X4 유형으로 변환합니다.
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, finalMatrix);

	// 절두체의 가까운 평면을 계산합니다.
	float x = (float)(matrix._14 + matrix._13);
	float y = (float)(matrix._24 + matrix._23);
	float z = (float)(matrix._34 + matrix._33);
	float w = (float)(matrix._44 + matrix._43);
	m_planes[0] = XMVectorSet(x, y, z, w);
	m_planes[0] = XMPlaneNormalize(m_planes[0]);

	// 절두체의 먼 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[1] = XMVectorSet(x, y, z, w);
	m_planes[1] = XMPlaneNormalize(m_planes[1]);

	// 절두체의 왼쪽 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[2] = XMVectorSet(x, y, z, w);
	m_planes[2] = XMPlaneNormalize(m_planes[2]);

	// 절두체의 오른쪽 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[3] = XMVectorSet(x, y, z, w);
	m_planes[3] = XMPlaneNormalize(m_planes[3]);

	// 절두체의 윗 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[4] = XMVectorSet(x, y, z, w);
	m_planes[4] = XMPlaneNormalize(m_planes[4]);

	// 절두체의 아래 평면을 계산합니다.
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