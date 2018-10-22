#pragma once
#include "Component.h"
#include "PhysicsClass.h"
#include <vector>

class MeshCollider;
class BoxCollider;
class SphereCollider;

class Collider :
	public Component
{
public:
	enum ColliderType
	{
		COL_TYPE_MESH,
		COL_TYPE_BOX,
		COL_TYPE_SPHERE
	};
	XMFLOAT3 offset;
private:
	bool added = false;
public:
	Collider();
	Collider(bool addToList);
	virtual ~Collider();
	virtual std::vector<XMFLOAT3> DetectCollisionWithMeshCollider(MeshCollider* col) = 0;
	virtual std::vector<XMFLOAT3> DetectCollisionWithBoxCollider(BoxCollider* col) = 0;
	virtual std::vector<XMFLOAT3> DetectCollisionWithSphereCollider(SphereCollider* col) = 0;
	virtual std::vector<RaycastHit> DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end) = 0;
	virtual Collider::ColliderType GetColliderType() = 0;
};

