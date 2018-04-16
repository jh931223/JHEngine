#pragma once
#include "Component.h"
#include <vector>
class Mesh;
class MeshRenderer;
class Voxel : public Component
{
public:
	Voxel();
	~Voxel();
	void Update() override;
	void OnStart() override;
	void Initialize();
	void CreateFaceUp(int x, int y, int z,byte, int&);
	void CreateFaceDown(int x, int y, int z, byte, int&);
	void CreateFaceRight(int x, int y, int z, byte, int&);
	void CreateFaceLeft(int x, int y, int z, byte, int&);
	void CreateFaceForward(int x, int y, int z, byte, int&);
	void CreateFaceBackward(int x, int y, int z, byte, int&);
	XMFLOAT2 GetUV(byte);
	byte GetBlock(int x, int y, int z);
	void UpdateMesh();
	Mesh* mesh;
	MeshRenderer* renderer;
private:
	void ReleaseChunks();
	void NewChunks();
	int width, height, depth;
	float unit;
	float tUnit;
	int tAmount;
	byte * * * chunks;
	std::vector<XMFLOAT3> vertices;
	std::vector<unsigned long> indices;
	std::vector<XMFLOAT2> uvs;
};


