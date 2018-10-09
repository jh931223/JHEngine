#pragma once
#include"Component.h"
class Mesh;
class TextureClass;
class Material;
class MeshRenderer : public Component
{
private:
	Mesh* mesh;
	Material* material;
public:
	MeshRenderer();
	~MeshRenderer();

	Mesh* GetMesh();
	void ReleaseMesh();
	void SetMesh(Mesh* _mesh);
	Material* GetMaterial();
	void SetMaterial(Material* _material);
	void Render(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX);
	// Component��(��) ���� ��ӵ�
	void Update();

	// Component��(��) ���� ��ӵ�
	void OnStart();
};

