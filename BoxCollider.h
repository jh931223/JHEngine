#pragma once
#include "Collider.h"
class BoxCollider :
	public Collider
{
public:
	XMFLOAT3 extends;
private:

public:
	BoxCollider();
	BoxCollider(bool addToList);
	~BoxCollider();

	// Collider��(��) ���� ��ӵ�
	virtual std::vector<XMFLOAT3> DetectCollisionWithMeshCollider(MeshCollider * col) override;
	virtual std::vector<XMFLOAT3> DetectCollisionWithBoxCollider(BoxCollider * col) override;
	virtual std::vector<XMFLOAT3> DetectCollisionWithSphereCollider(SphereCollider * col) override;
	virtual std::vector<RaycastHit> DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end) override;

	// Collider��(��) ���� ��ӵ�
	virtual Collider::ColliderType GetColliderType() override;
};

