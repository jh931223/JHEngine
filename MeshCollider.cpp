#include "stdafx.h"
#include "BoxCollider.h"
#include "MeshCollider.h"
#include "SphereCollider.h"
#include "MeshClass.h"


MeshCollider::MeshCollider()
{
}

MeshCollider::MeshCollider(bool addToList) :Collider(addToList)
{
}


MeshCollider::~MeshCollider()
{
}

std::vector<XMFLOAT3> MeshCollider::DetectCollisionWithMeshCollider(MeshCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<XMFLOAT3> MeshCollider::DetectCollisionWithBoxCollider(BoxCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<XMFLOAT3> MeshCollider::DetectCollisionWithSphereCollider(SphereCollider * col)
{
	return std::vector<XMFLOAT3>();
}


std::vector<RaycastHit> MeshCollider::DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end)
{
	std::vector<RaycastHit> output;
	auto verts = mesh->GetVertices();
	int vertSize = mesh->GetVertexCount();
	auto indices = mesh->GetIndices();
	int indSize = mesh->GetIndexCount();

	XMFLOAT3 startPos = (gameObject) ? transform()->GetWorldPosition() + offset : offset;

	XMFLOAT3 dir = end - start;
	XMFLOAT3 p1 = start, p2 = end;
	XMFLOAT3 p3;
	XMFLOAT3 N;
	float u;
	for (int i = 0; i < indSize-3; i += 3)
	{
		VertexBuffer vTemp[3];
		vTemp[0] = verts[indices[i]];
		vTemp[0].position += startPos;//// Orientation 수정필요
		vTemp[1] = verts[indices[i+1]];
		vTemp[1].position += startPos;
		vTemp[2] = verts[indices[i+2]];
		vTemp[2].position += startPos;
		N = CalcNormal(vTemp[0].position, vTemp[1].position, vTemp[2].position);
		p3 = vTemp[0].position;
		float b = Dot(N, (p2 - p1));
		if (b == 0)
			continue;
		u = Dot(N,(p3 - p1)) / b;
		if (u >= 0 || u <= 1)
		{
			XMFLOAT3 point= p1 + u * (p2 - p1);
			XMFLOAT3 v[3];
			v[0] = vTemp[1].position - vTemp[0].position;
			v[1] = vTemp[2].position - vTemp[1].position;
			v[2] = vTemp[0].position - vTemp[2].position;
			bool isInTriangle = true;
			for (int i = 0; i < 3; i++)
			{
				if (Dot(N,Cross(v[i], point - vTemp[i].position)) <= 0)
				{
					isInTriangle = false;
					break;
				}
			}
			if (isInTriangle)
			{
				RaycastHit hit;
				hit.collider = this;
				hit.point = point;
				hit.distance = GetDistance(hit.point, start);
				output.push_back(hit);
			}
			//작업중
			
		}
	}
	return output;
}

Collider::ColliderType MeshCollider::GetColliderType()
{
	return Collider::COL_TYPE_MESH;
}
