#include "stdafx.h"
#include "BoxCollider.h"
#include "MeshCollider.h"
#include "SphereCollider.h"

SphereCollider::SphereCollider()
{
}

SphereCollider::SphereCollider(bool addToList) :Collider(addToList)
{
}


SphereCollider::~SphereCollider()
{
}

std::vector<XMFLOAT3> SphereCollider::DetectCollisionWithMeshCollider(MeshCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<XMFLOAT3> SphereCollider::DetectCollisionWithBoxCollider(BoxCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<XMFLOAT3> SphereCollider::DetectCollisionWithSphereCollider(SphereCollider * col)
{
	std::vector<XMFLOAT3> result;
	XMFLOAT3 pos;
	if(gameObject)
		pos=transform()->GetWorldPosition() + transform()->right()*offset.x + transform()->up()*offset.y + transform()->forward()*offset.z;
	else pos = offset;
	XMFLOAT3 targetPos;
	if (col->gameObject)
		targetPos = col->transform()->GetWorldPosition() + col->transform()->right()*col->offset.x + col->transform()->up()*col->offset.y + col->transform()->forward()*col->offset.z;
	else
		targetPos = col->offset;
	XMFLOAT3 vec = targetPos - pos;
	float dis = vec.x*vec.x + vec.z*vec.z + vec.y + vec.y;
	if (dis <= radius+col->radius)
	{
		result.push_back(pos+Normalize3(vec)*radius);
	}
	return result;
}

std::vector<RaycastHit> SphereCollider::DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end)
{
	return std::vector<RaycastHit>();
}

Collider::ColliderType SphereCollider::GetColliderType()
{
	return Collider::COL_TYPE_SPHERE;
}
