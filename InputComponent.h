#pragma once
#include "Component.h"
class InputComponent :
	public Component
{
public:
	InputComponent();
	~InputComponent();
	void Update() override;
};

