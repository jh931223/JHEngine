#pragma once
#include"stdafx.h"
#include"VoxelComponent.h"
#include"MeshClass.h"
#include"MeshRenderer.h"
#include"SystemClass.h"
#include"TextureClass.h"
#include"Octree.h"
#include<vector>
#include<map>
Voxel::Voxel()
{
}

Voxel ::~Voxel()
{
	ReleaseChunks();
	mesh->SetVertices(NULL,0);
	mesh->SetIndices(NULL,0);
	mesh->Shutdown();
}

void Voxel::Update()
{
	if (transform()->parent)
	{
		//transform()->parent->RotateW(XMFLOAT3(0, 0.1f, 0));
	}
}

void Voxel::OnStart()
{
	Initialize();
}

void Voxel::Initialize()
{
	width = 32;
	height = 32;
	depth = 32;
	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;
	mesh = new Mesh;
	renderer->SetMesh(mesh);
	useMarchingCube = true;

	//LoadCube();
	LoadHeightMapFromRaw(1025,1025,512,"../JHEngine/data/heightmap.r16");


	octree = new Octree<int>(XMFLOAT3(32 * 0.5f, 32 * 0.5f, 32 * 0.5f), width, 6);
}
void Voxel::NewChunks()
{
	if (chunks)
		ReleaseChunks();
	chunks = new byte**[width];
	for (int i = 0; i < width; i++)
	{
		chunks[i] = new byte*[height];
		for (int j = 0; j < height; j++)
		{
			chunks[i][j] = new byte[depth];
			memset(chunks[i][j], 0, depth);
		}
	}
}
void Voxel::ReleaseChunks()
{
	if (chunks)
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				delete[] chunks[i][j];
				chunks[i][j] = NULL;
			}
			delete[] chunks[i];
			chunks[i] = NULL;
		}
		delete[] chunks;
		chunks = NULL;
	}
	if (mesh)
	{
		mesh->Shutdown();
		delete mesh;
	}
}

void Voxel::CreateFaceUp(int x, int y, int z, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = unit*0.5f;
	v1.position = XMFLOAT3(x*unit - offset, y*unit + offset, z*unit - offset);
	v2.position = XMFLOAT3(x*unit - offset, y*unit + offset, z*unit + offset);
	v3.position = XMFLOAT3(x*unit + offset, y*unit + offset, z*unit + offset);
	v4.position = XMFLOAT3(x*unit + offset, y*unit + offset, z*unit - offset);

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 3);


	XMFLOAT2 uv = GetUV(type);
	v1.texture = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.texture = XMFLOAT2(uv.x, uv.y);
	v3.texture = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.texture = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceDown(int x, int y, int z, byte type, int& faceCount)
{
	Mesh::VertexType v1, v2, v3, v4;
	float offset = unit * 0.5f;
	v1.position=XMFLOAT3(x*unit - offset, y*unit - offset, z*unit - offset);
	v2.position=XMFLOAT3(x*unit - offset, y*unit - offset, z*unit + offset);
	v3.position=XMFLOAT3(x*unit + offset, y*unit - offset, z*unit + offset);
	v4.position=XMFLOAT3(x*unit + offset, y*unit - offset, z*unit - offset);

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4 + 2);

	XMFLOAT2 uv = GetUV(type);
	v1.texture=XMFLOAT2(uv.x, uv.y + tUnit);
	v2.texture=XMFLOAT2(uv.x, uv.y);
	v3.texture=XMFLOAT2(uv.x + tUnit, uv.y);
	v4.texture=XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceRight(int x, int y, int z, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = unit * 0.5f;
	v1.position = XMFLOAT3(x*unit + offset, y*unit - offset, z*unit + offset);
	v2.position = XMFLOAT3(x*unit + offset, y*unit + offset, z*unit + offset);
	v3.position = XMFLOAT3(x*unit + offset, y*unit + offset, z*unit - offset);
	v4.position = XMFLOAT3(x*unit + offset, y*unit - offset, z*unit - offset);

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4 + 2);

	XMFLOAT2 uv = GetUV(type);
	v1.texture = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.texture = XMFLOAT2(uv.x, uv.y);
	v3.texture = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.texture = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceLeft(int x, int y, int z, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;

	float offset = unit * 0.5f;

	v1.position = XMFLOAT3(x*unit - offset, y*unit - offset, z*unit + offset);
	v2.position = XMFLOAT3(x*unit - offset, y*unit + offset, z*unit + offset);
	v3.position = XMFLOAT3(x*unit - offset, y*unit + offset, z*unit - offset);
	v4.position = XMFLOAT3(x*unit - offset, y*unit - offset, z*unit - offset);

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 3);

	XMFLOAT2 uv = GetUV(type);
	v1.texture = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.texture = XMFLOAT2(uv.x, uv.y);
	v3.texture = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.texture = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);

	faceCount++;
}
void Voxel::CreateFaceForward(int x, int y, int z, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = unit * 0.5f;
	v1.position=XMFLOAT3(x*unit - offset, y*unit- offset, z*unit+ offset);
	v2.position=XMFLOAT3(x*unit - offset, y*unit+ offset, z*unit+ offset);
	v3.position=XMFLOAT3(x*unit + offset, y*unit+ offset, z*unit+ offset);
	v4.position=XMFLOAT3(x*unit + offset, y*unit- offset, z*unit+ offset);

	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4 + 2);

	XMFLOAT2 uv = GetUV(type);
	v1.texture=XMFLOAT2(uv.x, uv.y + tUnit);
	v2.texture=XMFLOAT2(uv.x, uv.y);
	v3.texture=XMFLOAT2(uv.x + tUnit, uv.y);
	v4.texture=XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceBackward(int x, int y, int z, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = unit * 0.5f;
	v1.position=XMFLOAT3(x*unit- offset, y*unit- offset, z*unit- offset);
	v2.position=XMFLOAT3(x*unit- offset, y*unit+ offset, z*unit- offset);
	v3.position=XMFLOAT3(x*unit+ offset, y*unit+ offset, z*unit- offset);
	v4.position=XMFLOAT3(x*unit+ offset, y*unit- offset, z*unit- offset);

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4);

	XMFLOAT2 uv = GetUV(type);
	v1.texture=XMFLOAT2(uv.x, uv.y + tUnit);
	v2.texture=XMFLOAT2(uv.x, uv.y);
	v3.texture=XMFLOAT2(uv.x + tUnit, uv.y);
	v4.texture=XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceMarchingCube(int _case, int x, int y, int z, byte type)
{
	if (_case == 0 || _case == 255)
		return;
	float offset = unit * 0.5f;

	Mesh::VertexType vert[12] = { XMFLOAT3(0,0,0) };
	XMFLOAT2 uv = GetUV(type);
	for (int i = 0; i < 12; i++)
	{
		if ((edgeTable[_case] & (1 << i)) != 0)
		{
			vert[i].position = XMFLOAT3(x, y, z) + edgeMiddle[i] * offset;
		}
	}
	for (int i = 0; i<5; i++)
	{
		if (triTable[_case][i * 3] < 0)break;
		for (int j = 2; j >= 0; j--)
		{
			int edge = triTable[_case][i*3+j];
			vert[edge].texture=(j==0)? XMFLOAT2(uv.x + tUnit, uv.y + tUnit):(j==1)? XMFLOAT2(uv.x + tUnit*0.5f, uv.y): XMFLOAT2(uv.x, uv.y + tUnit);
			indices.push_back(vertices.size());
			vertices.push_back(vert[edge]);
		}
	}


}
void Voxel::GenerateMarchingCube()
{
	int faceCount = 0;
	unsigned short ***myCube;
	XMFLOAT3 cSize(width + 1, height + 1, depth + 1);
	myCube = new unsigned short**[cSize.x];
	for (int x = 0; x < cSize.x; x++)
	{
		myCube[x] = new unsigned short*[cSize.y];
		for (int y = 0; y < cSize.y; y++)
		{
			myCube[x][y] = new unsigned short[cSize.z];
			for (int z = 0; z < cSize.z; z++)
				myCube[x][y][z] = 0;
		}
	}
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				if (GetChunk(x, y, z))
				{
					myCube[x][y][z] |=             0b00100000;
					myCube[x + 1][y][z] |=         0b00010000;
					myCube[x][y + 1][z] |=         0b00000010;
					myCube[x + 1][y + 1][z] |=     0b00000001;
					myCube[x][y][z + 1] |=         0b01000000;
					myCube[x + 1][y][z + 1] |=     0b10000000;
					myCube[x][y + 1][z + 1] |=     0b00000100;
					myCube[x + 1][y + 1][z + 1] |= 0b00001000;
				}
			}
		}
	}
	int chunkTest = 1;
	for (int x = 0; x < cSize.x; x++)
	{
		for (int y = 0; y < cSize.y; y++)
		{
			for (int z = 0; z < cSize.z; z++)
			{
				CreateFaceMarchingCube(myCube[x][y][z], x, y, z, -1);
				chunkTest++;
				if (chunkTest >=tAmount)
					chunkTest = 1;
			}
			chunkTest++;
			if (chunkTest >= tAmount)
				chunkTest = 1;
		}
		chunkTest++;
		if (chunkTest >= tAmount)
			chunkTest = 1;
	}
	for (int x = 0; x < cSize.x; x++)
	{
		for (int y = 0; y < cSize.y; y++)
		{
			delete[] myCube[x][y];
		}
		delete[] myCube[x];
	}
	delete[] myCube;
}
void Voxel::GenerateVoxel()
{
	int faceCount = 0;
	byte myBlock;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				myBlock = GetChunk(x, y, z);
				if (myBlock == 0)
					continue;
				if (GetChunk(x - 1, y, z) == 0)//LEFT
				{
					CreateFaceLeft(x, y, z, myBlock, faceCount);
				}
				if (GetChunk(x + 1, y, z) == 0)//RIGHT
				{
					CreateFaceRight(x, y, z, myBlock, faceCount);
				}
				if (GetChunk(x, y + 1, z) == 0)//UP
				{
					CreateFaceUp(x, y, z, myBlock, faceCount);
				}
				if (GetChunk(x, y - 1, z) == 0)//DOWN
				{
					CreateFaceDown(x, y, z, myBlock, faceCount);
				}
				if (GetChunk(x, y, z + 1) == 0)//FORWARD
				{
					CreateFaceForward(x, y, z, myBlock, faceCount);
				}
				if (GetChunk(x, y, z - 1) == 0)//BACKWARD
				{
					CreateFaceBackward(x, y, z, myBlock, faceCount);
				}
			}
		}
	}
}


XMFLOAT2 Voxel::GetUV(byte type)
{
	if (type == -1)
		return XMFLOAT2(0, 0);
	return XMFLOAT2(tUnit*(type - 1), (int)(type / tAmount) * tUnit);
}

byte Voxel::GetChunk(int x, int y, int z)
{
	if (x >= width || y >= height || z >= depth)
		return 0;
	if (x < 0 || y < 0 || z < 0)
		return 0;
	return chunks[x][y][z];
}

void Voxel::UpdateMesh()
{
	vertices.clear();
	indices.clear();
	if (useMarchingCube)
		GenerateMarchingCube();
	else
		GenerateVoxel();
	mesh->SetVertices(&vertices[0],vertices.size());
	mesh->SetIndices(&indices[0], indices.size());
	mesh->RecalculateNormals();
	mesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
	//vertices.clear();
	//indices.clear();
	//mesh->SetVertices(NULL, 0);
	//mesh->SetIndices(NULL, 0);
}

void Voxel::LoadHeightMapFromRaw(int _width,int _depth,int _height,const char* filename)
{
	unsigned short** data = nullptr;
	height = _height;
	width = _width;
	depth = _depth;
	NewChunks();
	int top=0,bottom=0;
	ReadRawEX16(data, filename, width, depth,top,bottom);
	printf("%d, %d\n", top,bottom);
	float h = (float)height / 65536;
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			float convertY = ((float)data[x][depth-1-z] * h);
			for (int y = 0; (y < (int)roundl(convertY)); y++)
			{
				chunks[x][y][z] = 1;
			}
		}
	}
	for (int i = 0; i < width; i++)
		delete[] data[i];
	delete[] data;
	printf("%d개 생성 완료", width*depth*height);
	UpdateMesh();
}

void Voxel::LoadCube()
{
	NewChunks();
	int c = 1;
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			for (int y = 0; y < height; y++)
			{
				chunks[x][y][z] = c;
				c++;
				if (c == tAmount + 1)
					c = 1;
			}
			c++;
			if (c == tAmount + 1)
				c = 1;
		}
		c++;
		if (c == tAmount + 1)
			c = 1;
	}
	UpdateMesh();
}

void Voxel::ReadRawEX(unsigned char** &_srcBuf, const char* filename, int _width, int _height)
{
	unsigned char** srcBuf = new unsigned char*[_width];
	for (int i = 0; i < _width; i++)
	{
		srcBuf[i] = new unsigned char[_height];
		memset(srcBuf[i], 0, _height);
	}
	FILE* pInput = NULL;
	fopen_s(&pInput, filename, "rb");
	for (int i = 0; i < _width; i++)
	{
		fread_s(srcBuf[i], _height, 1, _height, pInput);
	}
	fclose(pInput);
	_srcBuf = srcBuf;
}
void Voxel::ReadRawEX16(unsigned short** &data, const char* filename, int _width, int _height,int& topY,int &bottomY)
{
	FILE* pInput = NULL;
	int size = _width * _height;
	unsigned short* _srcBuf = new unsigned short[size];
	fopen_s(&pInput, filename, "rb");
	if (fread(_srcBuf, sizeof(unsigned short), size, pInput) != size)
	{
		return;
	}
	fclose(pInput);
	data = new unsigned short*[_width];
	for (int i = 0; i < _width; i++)
		data[i] = new unsigned short[_height];
	topY = 0;
	bottomY = 0;
	for (int j = 0; j < _height; j++)
	{
		for (int i = 0; i < _width; i++)
		{
			int index = (_width * j) + i;
			data[i][j]= (int)_srcBuf[index];
			if (data[i][j] > topY)
				topY = data[i][j];
			if (data[i][j] < bottomY)
				bottomY = data[i][j];
		}
	}
}
