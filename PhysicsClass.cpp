#include "stdafx.h"
#include "PhysicsClass.h"
#include<algorithm>
#include"Collider.h"
#include"BoxCollider.h"
#include"SphereCollider.h"
#include"MeshCollider.h"


std::list<Collider*> PhysicsClass::colliderList;

PhysicsClass::PhysicsClass()
{
}


PhysicsClass::~PhysicsClass()
{
}

void PhysicsClass::AddCollider(Collider * collider)
{
	colliderList.push_back(collider);
}

void PhysicsClass::RemoveCollider(Collider * collider)
{
	auto iter=std::find(colliderList.begin(), colliderList.end(), collider);
	if(iter != colliderList.end())
		colliderList.erase(iter);
}

bool PhysicsClass::Raycast(XMFLOAT3 origin, XMFLOAT3 dir, float distance, RaycastHit& hitInfo)
{
	std::vector<RaycastHit> hitInfoList;
	float dis = distance+9999999;
	int index = -1;
	if (PhysicsClass::RaycastAll(origin, dir, distance, hitInfoList))
	{
		for (int i=0;i<hitInfoList.size();i++)
		{
			if (hitInfoList[i].distance < dis)
			{
				index = i;
				dis = hitInfoList[i].distance;
			}
		}
		if (index != -1)
		{
			hitInfo = hitInfoList[index];
			return true;
		}
	}
	return false;
}
bool PhysicsClass::SphereCastAll(XMFLOAT3 origin, XMFLOAT3 dir, float distance, float radius, std::vector<RaycastHit>& hitInfoList)
{
	SphereCollider collider(false);
	collider.radius = radius;
	collider.offset = origin;
	for (auto i : colliderList)
	{
		for (auto j : i->DetectCollisionWithSphereCollider(&collider))
		{
			RaycastHit hit;
			hit.collider = i;
			hit.point = j;
			hitInfoList.push_back(hit);
		}
	}
	return hitInfoList.size();
}
bool PhysicsClass::RaycastAll(XMFLOAT3 origin, XMFLOAT3 dir, float distance, std::vector<RaycastHit>& hitInfoList)
{
	for (auto i : colliderList)
	{
		auto result=i->DetectCollisionWithLine(origin, origin + dir * distance);
		for(auto j:result)
			hitInfoList.push_back(j);
	}
	return hitInfoList.size();
}
