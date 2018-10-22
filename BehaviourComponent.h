#pragma once
#include "Component.h"
class BehaviourComponent :
	public Component
{
public:
	BehaviourComponent();
	~BehaviourComponent();
	virtual void Update() {}
	virtual void LateUpdate() {}
	const int ComponentType() override;
};

