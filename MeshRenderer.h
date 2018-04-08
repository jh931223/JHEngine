#pragma once
#include"Component.h"
class Mesh;
class TextureClass;
class MaterialClass;
class MeshRenderer : public Component
{
private:
	Mesh* mesh;
	TextureClass* texture;
	MaterialClass* material;
public:
	MeshRenderer();
	~MeshRenderer();

	TextureClass* GetTexture();
	Mesh* GetMesh();
	void SetMesh(Mesh* _mesh);
	MaterialClass* GetMaterial();
	void SetMaterial(MaterialClass* _material);
	// Component��(��) ���� ��ӵ�
	void Update();

	// Component��(��) ���� ��ӵ�
	void OnStart();
};

