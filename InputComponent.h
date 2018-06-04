#pragma once
#include "Component.h"
class InputComponent :
	public Component
{
public:

	InputComponent();
	virtual ~InputComponent();
	void Update() override;
};

