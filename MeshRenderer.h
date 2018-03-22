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

	// Component을(를) 통해 상속됨
	void Update();

	// Component을(를) 통해 상속됨
	void OnStart();
};

