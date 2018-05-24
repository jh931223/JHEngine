#pragma once
#include"stdafx.h"
#include"VoxelComponent.h"
#include"MeshClass.h"
#include"MeshRenderer.h"
#include"SystemClass.h"
#include"TextureClass.h"
#include"CameraComponent.h"
#include"Octree.h"
#include<vector>
#include<map>
#include<time.h>
Voxel::Voxel()
{
}

Voxel ::~Voxel()
{
	ReleaseChunks();
	mesh->Shutdown();
}

void Voxel::Update()
{
	if (GetDistance(lastBasePosition, CameraComponent::mainCamera()->transform()->GetWorldPosition()) > 10)
	{
		lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
		if(useOctree)
			BuildOctree(width, lastBasePosition);
		UpdateMesh();
		chunkUpdated = false;
	}
	else if (chunkUpdated)
	{
		UpdateMesh();
		chunkUpdated = false;
	}
	if (transform()->parent)
	{
		//transform()->parent->RotateW(XMFLOAT3(0, 0.1f, 0));
	}
}

void Voxel::OnStart()
{
	transform()->SetPosition(XMFLOAT3(0, 0, 0));
	Initialize();
}

void Voxel::Initialize()
{
	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;
	useMarchingCube = true;
	useOctree = false;
	//LoadCube(32,32,32);
	SetLODLevel(0, 50);
	SetLODLevel(1, 100);
	SetLODLevel(2, 200);
	SetLODLevel(3, 300);
	lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	
	octreeType = 0;

	LoadCube(16, 16, 16);
	int h = ReadTXT("../JHEngine/height.txt");

	printf("TEST : %d\n", 1 << 4 + 2);

	//LoadHeightMapFromRaw(1025,1025,128,"../JHEngine/data/heightmap.r16");
	if(useOctree)
		BuildOctree(width, lastBasePosition);
	UpdateMesh();

}
void Voxel::NewChunks(int _width, int _depth, int _height)
{
	if (chunks)
		ReleaseChunks();
	height = _height;
	width = _width;
	depth = _depth;
	chunks = new byte**[width];
	for (int i = 0; i < width; i++)
	{
		chunks[i] = new byte*[height];
		for (int j = 0; j < height; j++)
		{
			chunks[i][j] = new byte[depth];
			for (int k = 0; k < depth; k++)
				chunks[i][j][k] = 0;
		}
	}
}
void Voxel::ReleaseChunks()
{
	if (chunks)
	{
		for (int i = 0; i < width; i++)
		{
			if (chunks[i]!=NULL)
			{
				for (int j = 0; j < height; j++)
				{
					delete[] chunks[i][j];
					chunks[i][j] = NULL;
				}
				delete[] chunks[i];
			}
			chunks[i] = NULL;
		}
		delete[] chunks;
		chunks = NULL;
	}
}
void Voxel::CreateFaceUp(float x, float y, float z, float _unit, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = _unit * 0.5f;
	v1.position = XMFLOAT3(x - offset, y + offset, z - offset);
	v2.position = XMFLOAT3(x - offset, y + offset, z + offset);
	v3.position = XMFLOAT3(x + offset, y + offset, z + offset);
	v4.position = XMFLOAT3(x + offset, y + offset, z - offset);

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
void Voxel::CreateFaceDown(float x, float y, float z, float _unit, byte type, int& faceCount)
{
	Mesh::VertexType v1, v2, v3, v4;
	float offset = _unit * 0.5f;
	v1.position = XMFLOAT3(x - offset, y - offset, z - offset);
	v2.position = XMFLOAT3(x - offset, y - offset, z + offset);
	v3.position = XMFLOAT3(x + offset, y - offset, z + offset);
	v4.position = XMFLOAT3(x + offset, y - offset, z - offset);

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
void Voxel::CreateFaceRight(float x, float y, float z, float _unit, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = _unit * 0.5f;
	v1.position = XMFLOAT3(x + offset, y - offset, z + offset);
	v2.position = XMFLOAT3(x + offset, y + offset, z + offset);
	v3.position = XMFLOAT3(x + offset, y + offset, z - offset);
	v4.position = XMFLOAT3(x + offset, y - offset, z - offset);

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
void Voxel::CreateFaceLeft(float x, float y, float z, float _unit, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;

	float offset = _unit * 0.5f;

	v1.position = XMFLOAT3(x - offset, y - offset, z + offset);
	v2.position = XMFLOAT3(x - offset, y + offset, z + offset);
	v3.position = XMFLOAT3(x - offset, y + offset, z - offset);
	v4.position = XMFLOAT3(x - offset, y - offset, z - offset);

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
void Voxel::CreateFaceForward(float x, float y, float z, float _unit, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = _unit * 0.5f;
	v1.position = XMFLOAT3(x - offset, y - offset, z + offset);
	v2.position = XMFLOAT3(x - offset, y + offset, z + offset);
	v3.position = XMFLOAT3(x + offset, y + offset, z + offset);
	v4.position = XMFLOAT3(x + offset, y - offset, z + offset);

	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4);
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
void Voxel::CreateFaceBackward(float x, float y, float z, float _unit, byte type, int& faceCount)
{

	Mesh::VertexType v1, v2, v3, v4;
	float offset = _unit * 0.5f;
	v1.position = XMFLOAT3(x - offset, y - offset, z - offset);
	v2.position = XMFLOAT3(x - offset, y + offset, z - offset);
	v3.position = XMFLOAT3(x + offset, y + offset, z - offset);
	v4.position = XMFLOAT3(x + offset, y - offset, z - offset);

	indices.push_back(faceCount * 4);
	indices.push_back(faceCount * 4 + 1);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 2);
	indices.push_back(faceCount * 4 + 3);
	indices.push_back(faceCount * 4);

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
void Voxel::CreateFaceMarchingCube(int _case, int x, int y, int z, int _unit, byte type)
{
	if (_case == 0 || _case == 255)
		return;
	float offset = _unit * 0.5f;

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
			int edge = triTable[_case][i * 3 + j];
			vert[edge].texture = (j == 0) ? XMFLOAT2(uv.x + tUnit, uv.y + tUnit) : (j == 1) ? XMFLOAT2(uv.x + tUnit * 0.5f, uv.y) : XMFLOAT2(uv.x, uv.y + tUnit);
			indices.push_back(vertices.size());
			vertices.push_back(vert[edge]);
		}
	}


}
void Voxel::GenerateOctreeFaces(Octree<int>::OctreeNode<int>* current,int& faceCount)
{
	if (current)
	{
		if(current->GetValue()>0)
		{
			if (!current->GetValue())
			{
				return;
			}
			int value = current->GetValue();
			XMFLOAT3 position = current->GetPosition();
			float size = current->GetCellSize();
			int depth = current->GetDepth();
			if (!octree->GetValueOfPosition(position + XMFLOAT3(0, size, 0), depth))
				CreateFaceUp(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(0, -size, 0), depth))
				CreateFaceDown(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(size, 0, 0), depth))
				CreateFaceRight(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(-size, 0, 0), depth))
				CreateFaceLeft(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(0, 0, size), depth))
				CreateFaceForward(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(0, 0, -size), depth))
				CreateFaceBackward(position.x, position.y, position.z, size, value, faceCount);
		}
		if (!current->IsLeaf())
		{
			for (int i = 0; i < 8; i++)
			{
				GenerateOctreeFaces(current->GetChild(i), faceCount);
			}
		}
	}
}
void Voxel::GenerateOctreeFaces2(Octree<int>::OctreeNode<int>* node, int& faceCount)
{
	if (node)
	{
		int value = node->GetValue();
		if (value > 0)
		{
			XMFLOAT3 position = node->GetPosition();
			float size = node->GetCellSize();
			if (!octree->GetValueOfPosition(position + XMFLOAT3(0, size, 0)))
				CreateFaceUp(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(0, -size, 0)))
				CreateFaceDown(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(size, 0, 0)))
				CreateFaceRight(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(-size, 0, 0)))
				CreateFaceLeft(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(0, 0, size)))
				CreateFaceForward(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPosition(position + XMFLOAT3(0, 0, -size)))
				CreateFaceBackward(position.x, position.y, position.z, size, value, faceCount);
		}
	}
}
void Voxel::GenerateOctreeFaces3(Octree<int>::OctreeNode<int>* node, int& faceCount)
{
	if (node)
	{
		int value = node->GetValue();
		int depth = node->GetDepth();
		if (value > 0)
		{
			XMFLOAT3 position = node->GetPosition();
			float size = node->GetCellSize();
			if (!octree->GetValueOfPositionByLevel(position + XMFLOAT3(0, size, 0),depth))
				CreateFaceUp(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPositionByLevel(position + XMFLOAT3(0, -size, 0), depth))
				CreateFaceDown(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPositionByLevel(position + XMFLOAT3(size, 0, 0), depth))
				CreateFaceRight(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPositionByLevel(position + XMFLOAT3(-size, 0, 0), depth))
				CreateFaceLeft(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPositionByLevel(position + XMFLOAT3(0, 0, size), depth))
				CreateFaceForward(position.x, position.y, position.z, size, value, faceCount);
			if (!octree->GetValueOfPositionByLevel(position + XMFLOAT3(0, 0, -size), depth))
				CreateFaceBackward(position.x, position.y, position.z, size, value, faceCount);
		}
	}
}
void Voxel::GenerateMarchingCubeFaces()
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
				CreateFaceMarchingCube(myCube[x][y][z], x, y, z,unit, -1);
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
void Voxel::GenerateVoxelFaces()
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
					CreateFaceLeft(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x + 1, y, z) == 0)//RIGHT
				{
					CreateFaceRight(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x, y + 1, z) == 0)//UP
				{
					CreateFaceUp(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x, y - 1, z) == 0)//DOWN
				{
					CreateFaceDown(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x, y, z + 1) == 0)//FORWARD
				{
					CreateFaceForward(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x, y, z - 1) == 0)//BACKWARD
				{
					CreateFaceBackward(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
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
	ULONG tick = GetTickCount64();
	if (useOctree)
	{
		UpdateOctreeMesh();
	}
	else
	{
		if (!useMarchingCube)
		{
			UpdateVoxelMesh();
		}
		else
		{
			UpdateMarchingCubeMesh();
		}
	}
	printf("Update Mesh : %dms\n", GetTickCount64() - tick);
}

void Voxel::UpdateVoxelMesh()
{
	Mesh* newMesh = new Mesh();
	if (mesh)
	{
		mesh->Shutdown();
		delete mesh;
		mesh = NULL;
	}
	GenerateVoxelFaces();
	newMesh->SetVertices(&vertices[0],vertices.size());
	newMesh->SetIndices(&indices[0], indices.size());
	newMesh->RecalculateNormals();
	newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
	vertices.clear();
	indices.clear();
	mesh = newMesh;
	renderer->SetMesh(newMesh);
	//vertices.clear();
	//indices.clear();
	//mesh->SetVertices(NULL, 0);
	//mesh->SetIndices(NULL, 0);
}
void Voxel::UpdateOctreeMesh()
{
	if (!octree)
	{
		return;
	}
	if (mesh)
	{
		mesh->Shutdown();
		delete mesh;
		mesh = NULL;
	}
	Mesh* newMesh = new Mesh();
	int faceCount = 0;
	int type = octreeType;
	if(type==0)
		GenerateOctreeFaces(octree->root, faceCount);
	else if(type==1)
	{
		std::vector<Octree<int>::OctreeNode<int>*> leafs;
		octree->root->GetLeafs(leafs);
		for (auto i : leafs)
		{
			GenerateOctreeFaces2(i, faceCount);
		}
	}
	else
	{
		for (int i = 0; i < octree->nodeSize; i++)
		{
			GenerateOctreeFaces3(octree->GetNormalizedNodes()[i], faceCount);
		}
	}

	newMesh->SetVertices(&vertices[0], vertices.size());
	newMesh->SetIndices(&indices[0], indices.size());
	newMesh->RecalculateNormals();
	newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
	vertices.clear();
	indices.clear();
	mesh = newMesh;
	renderer->SetMesh(newMesh);
	//vertices.clear();
	//indices.clear();
	//mesh->SetVertices(NULL, 0);
	//mesh->SetIndices(NULL, 0);
}

void Voxel::UpdateMarchingCubeMesh()
{
	if (mesh)
	{
		mesh->ShutdownBuffers();
		delete mesh;
		mesh = NULL;
	}
	Mesh* newMesh = new Mesh();
	GenerateMarchingCubeFaces();
	newMesh->SetVertices(&vertices[0], vertices.size());
	newMesh->SetIndices(&indices[0], indices.size());
	newMesh->RecalculateNormals();
	newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
	vertices.clear();
	indices.clear();
	mesh = newMesh;
	renderer->SetMesh(newMesh);
}

void Voxel::LoadHeightMapFromRaw(int _width,int _depth,int _height,const char* filename)
{
	unsigned short** data = nullptr;
	NewChunks(_width, _depth, _height);
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
				//octree->root->Insert(XMFLOAT3(x, y, z), 1);
				chunks[x][y][z] = 1;
			}
		}
	}
	for (int i = 0; i < width; i++)
		delete[] data[i];
	delete[] data;
	printf("%d byte chunk data 생성 완료\n", width*depth*height);
}

void Voxel::LoadCube(int _width, int _depth, int _height)
{
	NewChunks(_width, _depth, _height);
	int c = 1;
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			for (int y = 0; y < height; y++)
			{
				chunks[x][y][z] = 1;
			}
		}
	}
}

void Voxel::SetChunk(XMFLOAT3 position, BYTE value)
{
	position -= transform()->GetWorldPosition();
	int x = (int)(position.x / unit);
	int y = (int)(position.y / unit);
	int z = (int)(position.z / unit);
	if (x >= width || x < 0)
		return;
	if (z >= depth || z < 0)
		return;
	if (y >= height || y < 0)
		return;
	chunks[x][y][z] = value;
	if (useOctree)
	{
		int lodLevel = GetLODLevel(lastBasePosition, XMFLOAT3(x*unit + 0.1f, y*unit + 0.1f, z*unit + 0.1f));
		if (lodLevel < 0)
			lodLevel = octreeDepth;
		octree->root->Insert(XMFLOAT3(x*unit, y*unit, z*unit), chunks[x][y][z], lodLevel);
	}
	chunkUpdated = true;
}

void Voxel::BuildOctree(int _octreeLength, XMFLOAT3 basePosition)
{
	ULONG tick = GetTickCount64();
	//if (octreeType == 2)
	//{
	//	if (octree)
	//	{
	//		delete octree;
	//	}
	//	octreeLength = _octreeLength;
	//	int i = 0;
	//	int n = octreeLength;
	//	while ((n = n >> 1))
	//		++i;
	//	octreeDepth = i - 1;
	//	octree = new Octree<int>(XMFLOAT3(0, 0, 0), unit, octreeDepth,true);
	//	int lodLevel = 0;
	//	for (int x = 0; x < width; x++)
	//	{
	//		for (int y = 0; y < height; y++)
	//		{
	//			for (int z = 0; z < depth; z++)
	//			{
	//				lodLevel = GetLODLevel(basePosition, XMFLOAT3(x*unit, y*unit, z*unit));
	//				if (lodLevel>=0&&GetChunk(x, y, z))
	//				{
	//					int depth = octreeDepth - lodLevel;
	//					octree->Insert(XMFLOAT3(x*unit, y*unit, z*unit), chunks[x][y][z], depth);
	//				}
	//			}
	//		}
	//	}
	// return;
	//}
	bool merge = false;
	if (octreeLength != _octreeLength)
	{
		octreeLength = _octreeLength;
		int i = 0;
		int n = octreeLength;
		while ((n = n >> 1))
			++i;
		octreeDepth = i - 1;
		if (octree)
		{
			delete octree;
		}
		octree = new Octree<int>(XMFLOAT3(octreeLength * 0.5f, octreeLength * 0.5f, octreeLength * 0.5f), octreeLength, octreeDepth, merge);
	}
	else
	{
		octree->root->RemoveChilds();
	}
	int lodLevel = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				if (GetChunk(x, y, z))
				{
					XMFLOAT3 pos = XMFLOAT3(x*unit, y*unit, z*unit);
					lodLevel = GetLODLevel(basePosition, pos);
					if (lodLevel < 0)
						lodLevel = octreeDepth;
					octree->root->Insert(XMFLOAT3(x*unit, y*unit, z*unit), chunks[x][y][z], lodLevel, merge);
				}
			}
		}
	}
	printf("Build Octree : %dms\n", GetTickCount64() - tick);
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

int Voxel::GetLODLevel(XMFLOAT3 basePos, XMFLOAT3 targetPos)
{
	XMVECTOR v1, v2;
	float dis;
	v1 = XMVectorSet(basePos.x, basePos.y, basePos.z, 1);
	v2 = XMVectorSet(targetPos.x, targetPos.y, targetPos.z, 1);

	dis = XMVectorGetX(XMVector3Length(XMVectorSubtract(v2, v1)));
	for (int i = 0; i < 8; i++)
		if (LODDistance[i] >= dis)
		{
			return i;
		}
	return -1;
}

void Voxel::SetLODLevel(int level, float distance)
{
	if (level < 0 || level >= 8)
		return;
	LODDistance[level] = distance;
}

int Voxel::ReadTXT(const char * filename)
{
	FILE* pInput = NULL;
	int size =1;
	char str[100];
	fopen_s(&pInput, filename, "rb");
	fread(str, sizeof(str)-1, 1, pInput);
	fclose(pInput);
	return atoi(str);
}
