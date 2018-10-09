#pragma once
#include "SceneClass.h"
class MainScene :
	public SceneClass
{
public:
	MainScene();
	~MainScene();

	// SceneClass을(를) 통해 상속됨
	virtual void Setup() override;
};

