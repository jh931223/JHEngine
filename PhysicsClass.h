#pragma once
#include"Singleton.h"
#include<list>
#include<vector>

class Collider;

struct RaycastHit
{
	XMFLOAT3 point;
	XMFLOAT3 normal;
	float distance;
	Collider* collider;
};
class PhysicsClass
{
private:
	static std::list<Collider*> colliderList;
public:
	PhysicsClass();
	~PhysicsClass();
	static void AddCollider(Collider* collider);
	static void RemoveCollider(Collider* collider);
	static bool Raycast(XMFLOAT3 origin, XMFLOAT3 dir, float distance,RaycastHit& hitInfo);
	static bool SphereCastAll(XMFLOAT3 origin, XMFLOAT3 dir, float distance, float radius, std::vector<RaycastHit>& hitInfoList);
	static bool RaycastAll(XMFLOAT3 origin, XMFLOAT3 dir, float distance, std::vector<RaycastHit>& hitInfoList);
};

