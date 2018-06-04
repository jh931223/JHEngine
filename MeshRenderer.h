#pragma once
#include"Component.h"
class Mesh;
class TextureClass;
class Material;
class MeshRenderer : public Component
{
private:
	Mesh* mesh;
	TextureClass* texture;
	Material* material;
public:
	MeshRenderer();
	virtual ~MeshRenderer();

	TextureClass* GetTexture();
	Mesh* GetMesh();
	void SetMesh(Mesh* _mesh);
	Material* GetMaterial();
	void SetMaterial(Material* _material);
	void Render(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX);
	// Component��(��) ���� ��ӵ�
	void Update();

	// Component��(��) ���� ��ӵ�
	void OnStart();
};

