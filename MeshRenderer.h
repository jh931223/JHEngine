#pragma once
#include"Component.h"
class Mesh;
class TextureClass;
class MeshRenderer : public Component
{
public:
	Mesh* model;
	TextureClass* texture;
public:
	MeshRenderer();
	~MeshRenderer();

	// Component��(��) ���� ��ӵ�
	void Update();

	// Component��(��) ���� ��ӵ�
	void OnStart();
};

