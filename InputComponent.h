#pragma once
#include "BehaviourComponent.h"
class InputComponent :
	public BehaviourComponent
{
public:
	GameObject * cursorBox;
public:

	InputComponent();
	virtual ~InputComponent();
	void OnStart() override;
	void Update() override;

};

