#pragma once
#include"stdafx.h"
#include"VoxelComponent.h"
#include"MeshClass.h"
#include"MeshRenderer.h"
#include"SystemClass.h"
#include"TextureClass.h"
#include<vector>

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
}

void Voxel::OnStart()
{
	Initialize();
}

void Voxel::Initialize()
{
	width = 256;
	height = 64;
	depth = 256;
	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;
	mesh = new Mesh;
	renderer->SetMesh(mesh);

	//NewChunks();
	//int c = 1;
	//for (int x = 0; x < width; x++)
	//{
	//	for (int z = 0; z < depth; z++)
	//	{
	//		int r = rand() % height;
	//		for (int y = 0; y < height; y++)
	//		{
	//			if (y > r)
	//			{
	//				chunks[x][y][z] = 0;
	//				continue;
	//			}
	//			chunks[x][y][z] = c;
	//			c++;
	//			if (c == tAmount+1)
	//				c = 1;
	//		}
	//		c++;
	//		if (c == tAmount + 1)
	//			c = 1;
	//	}
	//	c++;
	//	if (c == tAmount + 1)
	//		c = 1;
	//}
	//UpdateMesh();

	LoadHeightMapFromRaw(1025,1025,"../JHEngine/data/heightmap.r16");
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
void Voxel::CreateFaceRight(int x, int y, int z, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = unit * 0.5f;
	v1.position=XMFLOAT3(x*unit+ offset, y*unit- offset, z*unit+ offset);
	v2.position=XMFLOAT3(x*unit+ offset, y*unit+ offset, z*unit+ offset);
	v3.position=XMFLOAT3(x*unit+ offset, y*unit+ offset, z*unit- offset);
	v4.position=XMFLOAT3(x*unit+ offset, y*unit- offset, z*unit- offset);

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
void Voxel::CreateFaceLeft(int x, int y, int z,byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;

	float offset = unit * 0.5f;

	v1.position = XMFLOAT3(x*unit - offset, y*unit - offset, z*unit + offset);
	v2.position = XMFLOAT3(x*unit- offset, y*unit+ offset, z*unit+ offset);
	v3.position = XMFLOAT3(x*unit- offset, y*unit+ offset, z*unit- offset);
	v4.position = XMFLOAT3(x*unit- offset, y*unit- offset, z*unit- offset);

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 3);

	XMFLOAT2 uv = GetUV(type);
	v1.texture=XMFLOAT2(uv.x, uv.y+tUnit);
	v2.texture = XMFLOAT2(uv.x, uv.y);
	v3.texture = XMFLOAT2(uv.x+tUnit, uv.y);
	v4.texture = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);

	faceCount++;
}

XMFLOAT2 Voxel::GetUV(byte type)
{
	return XMFLOAT2(tUnit*(type - 1), (int)(type / tAmount) * tUnit);
}

byte Voxel::GetBlock(int x, int y, int z)
{
	if (x >= width || y >= height || z >= depth)
		return 0;
	if (x < 0 || y < 0 || z < 0)
		return 0;
	return chunks[x][y][z];
}

void Voxel::UpdateMesh()
{
	int faceCount=0;
	int rightCount = 0;
	vertices.clear();
	indices.clear();
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				byte myBlock = GetBlock(x, y, z);
				if (myBlock == 0)
					continue;
				if (GetBlock(x - 1, y, z) == 0)//LEFT
				{
					CreateFaceLeft(x, y, z, myBlock, faceCount);
				}
				if (GetBlock(x + 1, y, z) == 0)//RIGHT
				{
					CreateFaceRight(x, y, z, myBlock, faceCount);
				}
				if (GetBlock(x, y + 1, z) == 0)//UP
				{
					CreateFaceUp(x, y, z, myBlock,faceCount);
				}
				if (GetBlock(x, y - 1, z) == 0)//DOWN
				{
					CreateFaceDown(x, y, z, myBlock, faceCount);
				}
				if (GetBlock(x, y, z + 1) == 0)//FORWARD
				{
					CreateFaceForward(x, y, z, myBlock, faceCount);
				}
				if (GetBlock(x, y, z - 1) == 0)//BACKWARD
				{
					CreateFaceBackward(x, y, z, myBlock, faceCount);
				}
			}
		}
	}
	mesh->SetVertices(&vertices[0],vertices.size());
	mesh->SetIndices(&indices[0], indices.size());
	mesh->RecalculateNormals();
	mesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
	//vertices.clear();
	//indices.clear();
	//mesh->SetVertices(NULL, 0);
	//mesh->SetIndices(NULL, 0);
}

void Voxel::LoadHeightMapFromRaw(int _width,int _depth,const char* filename)
{
	//unsigned char** data=nullptr;
	//height = 32;
	//width = _width;
	//depth = _depth;
	//NewChunks();
	//ReadRawEX(data, filename, width, depth);
	//for (int x = 0; x < width; x++)
	//{
	//	for (int z = 0; z < depth; z++)
	//	{
	//		float convertY = ((float)data[x][z]*((float)height/256.0f));
	//		for (int y = 0; (y < (int)floor(convertY)); y++)
	//		{
	//			chunks[x][y][z] = 1;
	//			
	//		}
	//	}
	//}
	//for (int i = 0; i < width; i++)
	//	delete[] data[i];
	//delete[] data;
	//printf("%d개 생성 완료", width*depth*height);
	unsigned short** data = nullptr;
	height = 512;
	width = _width;
	depth = _depth;
	NewChunks();
	int top=0,bottom=0;
	ReadRawEX16(data, filename, width, depth,top,bottom);
	printf("%d, %d\n", top,bottom);
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			float convertY = ((float)data[x][depth-1-z] * ((float)height / 65536));
			for (int y = 0; (y < (int)roundl(convertY)); y++)
			{
				chunks[x][y][z] = 9;
			}
		}
	}
	for (int i = 0; i < width; i++)
		delete[] data[i];
	delete[] data;
	printf("%d개 생성 완료", width*depth*height);
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