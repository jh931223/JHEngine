#pragma once
class GameObject;
class CTransform;
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
	virtual void Update() {};
	virtual void LateUpdate() {};
	CTransform* transform();
};

