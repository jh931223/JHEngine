#pragma once
#include "SceneClass.h"
class MainScene :
	public SceneClass
{
public:
	MainScene();
	~MainScene();

	// SceneClass��(��) ���� ��ӵ�
	virtual void Setup() override;
};

