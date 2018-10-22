#include "stdafx.h"
#include "BoxCollider.h"
#include "MeshCollider.h"
#include "SphereCollider.h"

BoxCollider::BoxCollider()
{
}

BoxCollider::BoxCollider(bool addToList) :Collider(addToList)
{
}


BoxCollider::~BoxCollider()
{
}

std::vector<XMFLOAT3> BoxCollider::DetectCollisionWithMeshCollider(MeshCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<XMFLOAT3> BoxCollider::DetectCollisionWithBoxCollider(BoxCollider * col)
{
	
	return std::vector<XMFLOAT3>();
}

std::vector<XMFLOAT3> BoxCollider::DetectCollisionWithSphereCollider(SphereCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<RaycastHit> BoxCollider::DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end)
{
	std::vector<RaycastHit> output;

	XMFLOAT3 center = (gameObject) ? transform()->GetWorldPosition() + offset : offset;
	XMFLOAT3 verts[8] = { center - extends,center + XMFLOAT3(extends.x,-extends.y,-extends.z) ,center + XMFLOAT3(-extends.x,-extends.y,extends.z),center + XMFLOAT3(extends.x,-extends.y,extends.z),
		center + XMFLOAT3(-extends.x,extends.y,-extends.z),center + XMFLOAT3(extends.x,extends.y,-extends.z) ,center + XMFLOAT3(-extends.x,extends.y,extends.z),center + extends };

	XMFLOAT3 p1 = start, p2 = end;
	XMFLOAT3 p3;
	XMFLOAT3 N;
	float u;
	int indices[6][4]=
	{
		{ 0,1,3,2 },
		{ 4,5,7,6 },
		{ 0,2,6,4 },
		{ 3,1,5,7 },
		{ 0,1,5,4 },
		{ 2,3,7,6 }

	};
	for (int i = 0; i < 6; i ++)
	{
		XMFLOAT3 vTemp[4];
		vTemp[0] = verts[indices[i][0]];
		vTemp[1] = verts[indices[i][1]];
		vTemp[2] = verts[indices[i][2]];
		vTemp[3] = verts[indices[i][3]];
		N = CalcNormal(vTemp[0], vTemp[1], vTemp[2]);
		p3 = vTemp[0];
		float b = Dot(N, (p2 - p1));
		if (b == 0)
			continue;
		u = Dot(N, (p3 - p1)) / b;
		if (u >= 0 || u <= 1)
		{
			XMFLOAT3 point = p1 + u * (p2 - p1);
			XMFLOAT3 v[4];
			v[0] = vTemp[1] - vTemp[0];
			v[1] = vTemp[2] - vTemp[1];
			v[2] = vTemp[3] - vTemp[2];
			v[3] = vTemp[0] - vTemp[3];
			bool isInQuad = true;
			for (int i = 0; i < 4; i++)
			{
				if (Dot(N, Cross(v[i], point - vTemp[i])) <= 0)
				{
					isInQuad = false;
					break;
				}
			}
			if (isInQuad)
			{
				RaycastHit hit;
				hit.collider = this;
				hit.point = point;
				hit.distance = GetDistance(hit.point, start);
				output.push_back(hit);
			}
			//ÀÛ¾÷Áß

		}
	}
	return output;
}

Collider::ColliderType BoxCollider::GetColliderType()
{
	return COL_TYPE_BOX;
}
