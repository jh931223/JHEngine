#pragma once
#include"stdafx.h"
#include"VoxelComponent.h"
#include"MeshClass.h"
#include"MeshRenderer.h"
#include"SystemClass.h"
#include<vector>

Voxel::Voxel()
{
}

Voxel ::~Voxel()
{
	ReleaseChunks();
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
	width = 100;
	height = 2;
	depth = 100;
	unit = 3.0f;
	tUnit = 0.25f;
	tAmount = 4;
	NewChunks();
	int c = 1;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				chunks[x][y][z] = c;
				c++;
				if (c == tAmount+1)
					c = 0;
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
void Voxel::NewChunks()
{
	if (chunks)
		ReleaseChunks();
	chunks = new byte**[width];
	for (int i = 0; i < width; i++)
	{
		chunks[i] = new byte*[height];
		for (int j = 0; j < height; j++)
			chunks[i][j] = new byte[depth];
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
}

void Voxel::CreateFaceUp(int x, int y, int z, byte type, int& faceCount)
{
	float offset = unit*0.5f;
	vertices.push_back(XMFLOAT3(x*unit- offset, y*unit+ offset, z*unit- offset));
	vertices.push_back(XMFLOAT3(x*unit- offset, y*unit+ offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit+ offset, y*unit+ offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit+ offset, y*unit+ offset, z*unit- offset));

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 3);


	XMFLOAT2 uv = GetUV(type);
	uvs.push_back(XMFLOAT2(uv.x, uv.y + tUnit));
	uvs.push_back(XMFLOAT2(uv.x, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y + tUnit));
	faceCount++;
}
void Voxel::CreateFaceDown(int x, int y, int z, byte type, int& faceCount)
{
	float offset = unit * 0.5f;
	vertices.push_back(XMFLOAT3(x*unit - offset, y*unit - offset, z*unit - offset));
	vertices.push_back(XMFLOAT3(x*unit - offset, y*unit - offset, z*unit + offset));
	vertices.push_back(XMFLOAT3(x*unit + offset, y*unit - offset, z*unit + offset));
	vertices.push_back(XMFLOAT3(x*unit + offset, y*unit - offset, z*unit - offset));

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4 + 2);

	XMFLOAT2 uv = GetUV(type);
	uvs.push_back(XMFLOAT2(uv.x, uv.y + tUnit));
	uvs.push_back(XMFLOAT2(uv.x, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y + tUnit));

	faceCount++;
}
void Voxel::CreateFaceForward(int x, int y, int z, byte type, int& faceCount)
{
	float offset = unit * 0.5f;
	vertices.push_back(XMFLOAT3(x*unit - offset, y*unit- offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit - offset, y*unit+ offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit + offset, y*unit+ offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit + offset, y*unit- offset, z*unit+ offset));

	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4 + 2);

	XMFLOAT2 uv = GetUV(type);
	uvs.push_back(XMFLOAT2(uv.x, uv.y + tUnit));
	uvs.push_back(XMFLOAT2(uv.x, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y + tUnit));

	faceCount++;
}
void Voxel::CreateFaceBackward(int x, int y, int z, byte type, int& faceCount)
{
	float offset = unit * 0.5f;
	vertices.push_back(XMFLOAT3(x*unit- offset, y*unit- offset, z*unit- offset));
	vertices.push_back(XMFLOAT3(x*unit- offset, y*unit+ offset, z*unit- offset));
	vertices.push_back(XMFLOAT3(x*unit+ offset, y*unit+ offset, z*unit- offset));
	vertices.push_back(XMFLOAT3(x*unit+ offset, y*unit- offset, z*unit- offset));

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4);

	XMFLOAT2 uv = GetUV(type);
	uvs.push_back(XMFLOAT2(uv.x, uv.y + tUnit));
	uvs.push_back(XMFLOAT2(uv.x, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y + tUnit));

	faceCount++;
}
void Voxel::CreateFaceRight(int x, int y, int z, byte type, int& faceCount)
{
	float offset = unit * 0.5f;
	vertices.push_back(XMFLOAT3(x*unit+ offset, y*unit- offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit+ offset, y*unit+ offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit+ offset, y*unit+ offset, z*unit- offset));
	vertices.push_back(XMFLOAT3(x*unit+ offset, y*unit- offset, z*unit- offset));

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4 + 2);

	XMFLOAT2 uv = GetUV(type);
	uvs.push_back(XMFLOAT2(uv.x, uv.y + tUnit));
	uvs.push_back(XMFLOAT2(uv.x, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y + tUnit));

	faceCount++;
}
void Voxel::CreateFaceLeft(int x, int y, int z,byte type, int& faceCount)
{
	float offset = unit * 0.5f;
	vertices.push_back(XMFLOAT3(x*unit- offset, y*unit- offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit- offset, y*unit+ offset, z*unit+ offset));
	vertices.push_back(XMFLOAT3(x*unit- offset, y*unit+ offset, z*unit- offset));
	vertices.push_back(XMFLOAT3(x*unit- offset, y*unit- offset, z*unit- offset));

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 3);

	XMFLOAT2 uv = GetUV(type);
	uvs.push_back(XMFLOAT2(uv.x, uv.y+tUnit));
	uvs.push_back(XMFLOAT2(uv.x, uv.y));
	uvs.push_back(XMFLOAT2(uv.x+tUnit, uv.y));
	uvs.push_back(XMFLOAT2(uv.x + tUnit, uv.y + tUnit));

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
					rightCount++;
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
	printf("rC : %d\n", rightCount);
	if (renderer->GetMesh())
	{
		renderer->GetMesh()->Shutdown();
		delete renderer->GetMesh();
	}
	Mesh* m = new Mesh;
	renderer->SetMesh(m);
	m->SetVertices(&vertices[0],vertices.size());
	printf("verts : %d ", vertices.size());
	m->SetIndices(&indices[0], indices.size());
	printf("faces : %d \n", indices.size()/3/2);
	m->SetUVs(&uvs[0]);
	m->RecalculateNormals();
	m->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
}
