#pragma once
#include "GameObject.h"
#include "Transform.h"
class Component : public AlignedAllocationPolicy<16>
{
public:
	GameObject* gameObject;
	bool enabled=true;
public:
	Component() {  }
	Component(GameObject* _parent) :gameObject(_parent) {}
	virtual ~Component() {}
	virtual void OnStart() {};
	virtual const int ComponentType()
	{
		return 0;
	}
	CTransform* transform();
};

