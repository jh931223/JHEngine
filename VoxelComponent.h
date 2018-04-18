#pragma once
#include "Component.h"
#include <vector>
#include "MeshClass.h"
class MeshRenderer;
class TextureClass;
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
	void LoadHeightMapFromRaw(int,int,const char*);
	XMFLOAT2 GetUV(byte);
	byte GetBlock(int x, int y, int z);
	void UpdateMesh();
	Mesh* mesh;
	MeshRenderer* renderer;
private:
	void ReleaseChunks();
	void NewChunks();
	void ReadRawEX(unsigned char** &_srcBuf, const char* filename, int _width, int _height);
	void ReadRawEX16(unsigned short** &_srcBuf, const char* filename, int _width, int _height,int&,int&);
	int width, height, depth;
	float unit;
	float tUnit;
	int tAmount;
	byte * * * chunks;
	std::vector<Mesh::VertexType> vertices;
	std::vector<unsigned long> indices;
};


