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
	// Component을(를) 통해 상속됨
	void Update();

	// Component을(를) 통해 상속됨
	void OnStart();
};

