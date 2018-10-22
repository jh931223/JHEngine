#pragma once
#include "Collider.h"

class Mesh;

class MeshCollider :
	public Collider
{
public:
	Mesh * mesh;
public:
	MeshCollider();
	MeshCollider(bool addToList);
	~MeshCollider();

	// Collider을(를) 통해 상속됨
	virtual std::vector<XMFLOAT3> DetectCollisionWithMeshCollider(MeshCollider * col) override;
	virtual std::vector<XMFLOAT3> DetectCollisionWithBoxCollider(BoxCollider * col) override;
	virtual std::vector<XMFLOAT3> DetectCollisionWithSphereCollider(SphereCollider * col) override;
	virtual std::vector<RaycastHit> DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end) override;

	// Collider을(를) 통해 상속됨
	virtual Collider::ColliderType GetColliderType() override;
};

