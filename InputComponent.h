#pragma once
#include "BehaviourComponent.h"
class InputComponent :
	public BehaviourComponent
{
public:
	GameObject * box;
public:

	InputComponent();
	virtual ~InputComponent();
	void Update() override;

};

