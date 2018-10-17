#pragma once
#include"Component.h"
class Mesh;
class TextureClass;
class Material;
class MeshRenderer : public Component
{
	struct MeshBoundary
	{
		int size=16;
		XMFLOAT3 centerOffset=XMFLOAT3(0,0,0);
	};
private:
	Mesh* mesh;
	Material* material;
public:
	MeshBoundary boundary;
public:
	MeshRenderer();
	~MeshRenderer();

	Mesh* GetMesh();
	void ReleaseMesh();
	void SetMesh(Mesh* _mesh);
	Material* GetMaterial();
	void SetMaterial(Material* _material);
	void Render(ID3D11DeviceContext*, XMMATRIX, XMMATRIX);
	// Component��(��) ���� ��ӵ�
	void Update();

	// Component��(��) ���� ��ӵ�
	void OnStart();
};

class Frustum
{
	static XMVECTOR m_planes[6];
public:
	static bool isLockFrustum;
	static int frustumCulled;
	static int drawnVertex;
public:
	static void ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix);
	static bool FrustumCheckCube(float xCenter, float yCenter, float zCenter, float radius);
	static bool FrustumCheckSphere(float xCenter, float yCenter, float zCenter, float radius);
};

