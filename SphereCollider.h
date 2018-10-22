#pragma once
#include "Collider.h"
class SphereCollider :
	public Collider
{
public:
	float radius;
private:

public:
	SphereCollider();
	SphereCollider(bool addToList);
	~SphereCollider();

	// Collider을(를) 통해 상속됨
	virtual std::vector<XMFLOAT3> DetectCollisionWithMeshCollider(MeshCollider * col) override;
	virtual std::vector<XMFLOAT3> DetectCollisionWithBoxCollider(BoxCollider * col) override;
	virtual std::vector<XMFLOAT3> DetectCollisionWithSphereCollider(SphereCollider * col) override;
	virtual std::vector<RaycastHit> DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end) override;

	// Collider을(를) 통해 상속됨
	virtual Collider::ColliderType GetColliderType() override;
};

