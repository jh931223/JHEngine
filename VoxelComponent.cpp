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
#include"PerlinNoise.h"
#include<amp.h>
#include<amp_graphics.h>
#include"SystemClass.h"
#include"MaterialClass.h"
#include"StructuredBuffer.h"
#include"ResourcesClass.h"

using namespace concurrency;
using namespace concurrency::graphics;
using namespace concurrency::direct3d;

Voxel::Voxel()
{
}

Voxel ::~Voxel()
{
	ReleaseChunks();
	mesh->Shutdown();
	Buf_chunkData->Release();
}

void Voxel::Update()
{
	//if (GetDistance(lastBasePosition, CameraComponent::mainCamera()->transform()->GetWorldPosition()) > 10)
	//{
	//	lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	//	if(useOctree)
	//		BuildOctree(width, lastBasePosition);
	//	UpdateMesh();
	//	chunkUpdated = false;
	//}
	//else if (chunkUpdated)
	//{
	//	UpdateMesh();
	//	chunkUpdated = false;
	//}
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

	ULONG tick = GetTickCount64();
	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;
	useMarchingCube = true;
	useOctree = false;
	octreeMerge = false;
	useGPGPU = false;
	useGeometry = true;
	
	SetLODLevel(0, 5000000);
	SetLODLevel(1, 100);
	SetLODLevel(2, 200);
	SetLODLevel(3, 300);
	lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	
	octreeType = 0;

	//LoadCube(32, 32, 32);
	LoadPerlin(128, 128, 128,64, 0.3);
	int h = ReadTXT("../JHEngine/height.txt");




	//LoadHeightMapFromRaw(1025,128,1025,"../JHEngine/data/heightmap.r16");
	if(useOctree&&!octree)
		BuildOctree(width, lastBasePosition);
	UpdateMesh();
	printf("Init time : %dms\n", GetTickCount64() - tick);

}
void Voxel::NewChunks(int _width, int _height,int _depth)
{
	if (chunksArray)
		ReleaseChunks();
	height = _height;
	width = _width;
	depth = _depth;
	chunksArray = new byte[width*height*depth]{ 0, };
	//memset(chunksArray, 0, sizeof(byte)*width*height*depth);
}
void Voxel::ReleaseChunks()
{
	if (chunksArray)
	{
		delete[] chunksArray;
	}
	chunksArray = 0;
}
void Voxel::CreateFaceUp(float x, float y, float z, float _unit, byte type, int& faceCount)
{

	VertexBuffer v1, v2, v3, v4;
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
	VertexBuffer v1, v2, v3, v4;
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

	VertexBuffer v1, v2, v3, v4;
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

	VertexBuffer v1, v2, v3, v4;

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

	VertexBuffer v1, v2, v3, v4;
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

	VertexBuffer v1, v2, v3, v4;
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
void Voxel::CreateFaceMarchingCube(int _case, float x, float y, float z, int _unit, byte type)
{
	if (_case == 0 || _case == 255)
		return;
	float offset = _unit * 0.5f;

	VertexBuffer vert[12] = { XMFLOAT3(0,0,0) };
	XMFLOAT2 uv = GetUV(type);
	for (int i = 0; i < 12; i++)
	{
		if ((edgeTable[_case] & (1 << i)) != 0)
		{
			vert[i].position = XMFLOAT3(x, y, z) + mcVertexOffset[i] * offset;
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
		int index = vertices.size() - 3;
		//CalcNormal(vertices[index], vertices[index + 1], vertices[index + 2]);
	}
}
void Voxel::AddMarchingCase(int* marching,int x, int y, int z, int w, int h, int d, int _case)
{
	int idx = (x)+(y)* w + (z)* h * d;
	if (marching[idx] == 0)
	{
		VertexBuffer vert;
		vert.position = XMFLOAT3(x, y, z) - XMFLOAT3((w - 1)*0.5f, (h - 1)*0.5f, (d - 1)*0.5f);
		//indices.push_back(vertices.size());
		vertices.push_back(vert);
	}
	marching[idx] |= _case;
}
void Voxel::GenerateOctreeFaces(OctreeNode<int>* current,int& faceCount)
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
void Voxel::GenerateOctreeFaces2(OctreeNode<int>* node, int& faceCount)
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
void Voxel::GenerateMarchingCubeFaces()
{
	int *myCube;
	int size[3]{ width + 1,height + 1,depth + 1 };
	myCube = new int[size[0]*size[1]*size[2]]{ 0, };
	ULONG time = GetTickCount();
	if (useGeometry)
	{
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				for (int z = 0; z < depth; z++)
				{
					if (GetChunk(x, y, z))
					{
						AddMarchingCase(myCube, x, y, z, size[0], size[1], size[2], 32);
						AddMarchingCase(myCube, x + 1, y, z, size[0], size[1], size[2], 16);
						AddMarchingCase(myCube, x, y + 1, z, size[0], size[1], size[2], 2);
						AddMarchingCase(myCube, x + 1, y + 1, z, size[0], size[1], size[2], 1);
						AddMarchingCase(myCube, x, y, z + 1, size[0], size[1], size[2], 64);
						AddMarchingCase(myCube, x + 1, y, z + 1, size[0], size[1], size[2], 128);
						AddMarchingCase(myCube, x, y + 1, z + 1, size[0], size[1], size[2], 4);
						AddMarchingCase(myCube, x + 1, y + 1, z + 1, size[0], size[1], size[2], 8);
					}
				}
			}
		}
		int nums = size[0] * size[1] * size[2];
		Material* material = renderer->GetMaterial();
		material->GetParams()->SetInt("vertCount", vertices.size());
		material->GetParams()->SetFloat3("startPosition", vertices[0].position);
		material->GetParams()->SetInt("length", size[0]);
		ID3D11Device* device = SystemClass::GetInstance()->GetDevice();
		//ID3D11ShaderResourceView* srv = NULL;
		//CreateSRV<int>(device, nums, srv, myCube);

		material->GetParams()->SetSRV("chunkData", new StructuredBuffer(device,sizeof(int),nums,myCube));
		printf("Marching Cube Create Buffer ( Geometry ): %d ms\n", GetTickCount() - time);
		delete[] myCube;
		return;
	}
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				if (GetChunk(x, y, z))
				{
					myCube[(x)+(y)* size[0] + (z)* size[0] * size[1]] |= 0b00100000;
					myCube[(x + 1) + y * size[0] + z * size[0] * size[1]] |= 0b00010000;
					myCube[x + (y + 1)*size[0] + z * size[0] * size[1]] |= 0b00000010;
					myCube[(x + 1) + (y + 1)* size[0] + (z)* size[0] * size[1]] |= 0b00000001;
					myCube[(x)+(y)* size[0] + (z + 1)* size[0] * size[1]] |= 0b01000000;
					myCube[(x + 1) + (y)* size[0] + (z + 1)* size[0] * size[1]] |= 0b10000000;
					myCube[(x)+(y + 1)* size[0] + (z + 1)* size[0] * size[1]] |= 0b00000100;
					myCube[(x + 1) + (y + 1)* size[0] + (z + 1)* size[0] * size[1]] |= 0b00001000;
				}
			}
		}
	}
	if (useGPGPU)
	{
		const size_t SIZE = size[0] * size[1] * size[2] * 3 * 5;

		array_view<const int, 3> cubes(size[0], size[1], size[2], myCube);

		//float_4* v=new float_4[SIZE];
		//float_2* t = new float_2[SIZE];
		//float_3* n = new float_3[SIZE];

		array_view<float_4, 1> gVertBuffer(SIZE);
		array_view<float_2, 1> gTexBuffer(SIZE);
		array_view<float_3, 1> gNormalBuffer(SIZE);

		const float_3 edgeMiddleF3[12] =
		{
			float_3(0,-1,1),
			float_3(1,-1,0),
			float_3(0,-1,-1),
			float_3(-1,-1,0),

			float_3(0,1,1),
			float_3(1,1,0),
			float_3(0,1,-1),
			float_3(-1,1,0),

			float_3(-1,0,1),
			float_3(1,0,1),
			float_3(1,0,-1),
			float_3(-1,0,-1)
		};
		array_view<const float_3, 1> edge(12, edgeMiddleF3);
		array_view<const int, 2> triT(16, 256, triTableN);
		array_view<const int, 1> edgeT(256, edgeTable);
		array_view<const int, 1> _size(3, size);
		array_view<XMFLOAT3, 1> test(3);
		int tU = tUnit;
		//gVertBuffer.discard_data();
		//gTexBuffer.discard_data();
		//gNormalBuffer.discard_data();
		parallel_for_each(cubes.extent, [=](index<3> idxs)restrict(amp)
		{
			int _case = cubes[idxs[0]][idxs[1]][idxs[2]];
			if (_case == 0 || _case == 255)
				return;
			float offset = 1.0f * 0.5f;
			float_3 _verts[12];
			for (int i = 0; i < 12; i++)
			{
				if ((edgeT[_case] & (1 << i)) != 0)
				{
					float_3 newPos = float_3(idxs[0], idxs[1], idxs[2]) + float_3(edge[i].x, edge[i].y, edge[i].z) * offset;
					_verts[i] = newPos;
				}
			}
			int idx = idxs[0] + idxs[1] * size[0] + idxs[2] * size[0] * size[1];
			for (int i = 0; i < 5; i++)
			{
				for (int j = 2; j >= 0; j--)
				{
					int totalID = idx * 15 + (3 * i + j);
					if (triT[_case][i * 3] < 0)
					{
						gVertBuffer[totalID] = float_4(0, 0, 0, 0);
						continue;
					}
					int edge = triT[_case][i * 3 + j];
					gTexBuffer[totalID] = (j == 0) ? float_2(0, 0) : (j == 1) ? float_2(1 * 0.5f, 0) : float_2(0, 1);
					gVertBuffer[totalID] = float_4(_verts[edge].x, _verts[edge].y, _verts[edge].z, 1);
					gNormalBuffer[totalID] = float_3(0, 0, 0);
				}
			}
		});
		printf("Marching Cube Create Buffer ( GPU ): %d ms\n", GetTickCount() - time);
		for (int i = 0; i < SIZE; i++)
		{
			if (gVertBuffer[i].w == 1)
			{
				VertexBuffer vert;
				vert.position = XMFLOAT3(gVertBuffer[i].x, gVertBuffer[i].y, gVertBuffer[i].z);
				vert.texture = XMFLOAT2(gTexBuffer[i].x, gTexBuffer[i].y);
				indices.push_back(vertices.size());
				vertices.push_back(vert);
			}
		}
	}
	else
	{
		for (int x = 0; x < size[0]; x++)
		{
			for (int y = 0; y < size[1]; y++)
			{
				for (int z = 0; z < size[2]; z++)
				{
					CreateFaceMarchingCube(myCube[(x)+(y)* size[0] + (z)* size[0] * size[1]], x, y, z, unit, -1);
				}
			}
		}
		printf("Marching Cube Create Buffer ( CPU ): %d ms\n", GetTickCount() - time);
	}
	delete[] myCube;
}
void Voxel::GenerateMarchingCubeOctreeFaces()
{

	Octree<short>* myCubeOctree = new Octree<short>(transform()->GetWorldPosition(), octree->size*2, octree->depth + 1);

	std::vector<OctreeNode<int>*> leafs;
	octree->root->GetLeafs(leafs);
	for (auto i : leafs)
	{
		if (i->GetValue())
		{
			int x = i->GetPosition().x;
			int y = i->GetPosition().y;
			int z = i->GetPosition().z;

			OctreeNode<short>* n;
			n = OctreeNode<short>::Subdivide(myCubeOctree->root, XMFLOAT3(x, y, z), myCubeOctree->depth);
			n->SetValue(n->GetValue() | 0b00100000);
			n = OctreeNode<short>::Subdivide(myCubeOctree->root, XMFLOAT3(x+1, y, z), myCubeOctree->depth);
			n->SetValue(n->GetValue() | 0b00010000);
			n = OctreeNode<short>::Subdivide(myCubeOctree->root, XMFLOAT3(x, y+1, z), myCubeOctree->depth);
			n->SetValue(n->GetValue() | 0b00000010);
			n = OctreeNode<short>::Subdivide(myCubeOctree->root, XMFLOAT3(x+1, y+1, z), myCubeOctree->depth);
			n->SetValue(n->GetValue() | 0b00000001);
			n = OctreeNode<short>::Subdivide(myCubeOctree->root, XMFLOAT3(x, y, z+1), myCubeOctree->depth);
			n->SetValue(n->GetValue() | 0b01000000);
			n = OctreeNode<short>::Subdivide(myCubeOctree->root, XMFLOAT3(x+1, y, z+1), myCubeOctree->depth);
			n->SetValue(n->GetValue() | 0b10000000);
			n = OctreeNode<short>::Subdivide(myCubeOctree->root, XMFLOAT3(x, y+1, z+1), myCubeOctree->depth);
			n->SetValue(n->GetValue() | 0b00000100);
			n = OctreeNode<short>::Subdivide(myCubeOctree->root, XMFLOAT3(x+1, y+1, z+1), myCubeOctree->depth);
			n->SetValue(n->GetValue() | 0b00001000);
		}
	}

	std::vector<OctreeNode<short>*> cleafs;
	myCubeOctree->root->GetLeafs(cleafs);
	for (auto i : cleafs)
	{
		CreateFaceMarchingCube(i->GetValue(), i->GetPosition().x , i->GetPosition().y , i->GetPosition().z , unit, 1);
	}
	delete myCubeOctree;

}
void Voxel::GenerateMarchingCubeOctreeFaces2()
{
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

	std::vector<OctreeNode<int>*> leafs;
	octree->root->GetLeafs(leafs);
	for (auto i : leafs)
	{
		if (i->GetValue())
		{
			XMFLOAT3 newPos = CovertToChunkPos(i->GetPosition());
			int x = newPos.x;
			int y = newPos.y;
			int z = newPos.z;

			/*
			    ③ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ②
			   /l                         /l
			  / l                        / l
			 /  l                       /  l
			/   l                      /   l
		   /    l                     /    l
		  /     l                    /     l
		 ①ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡⓞ      l
		 l      l                   I      l
		 l      ⑦__________________l_____⑥
		 l     /                    l     /
		 l    /                     l    /
		 l   /                      l   /
		 l  /                       l  /
		 l /                        l /
		 l/                         l/
		 ⑤ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ④
			
			
			
			
			*/

			myCube[x][y][z] |= 0b00100000;
			myCube[x + 1][y][z] |= 0b00010000;
			myCube[x][y + 1][z] |= 0b00000010;
			myCube[x + 1][y + 1][z] |= 0b00000001;
			myCube[x][y][z + 1] |= 0b01000000;
			myCube[x + 1][y][z + 1] |= 0b10000000;
			myCube[x][y + 1][z + 1] |= 0b00000100;
			myCube[x + 1][y + 1][z + 1] |= 0b00001000;
		}
	}
	for (int x = 0; x < cSize.x; x++)
	{
		for (int y = 0; y < cSize.y; y++)
		{
			for (int z = 0; z < cSize.z; z++)
			{
				CreateFaceMarchingCube(myCube[x][y][z], x, y, z, unit, -1);
			}
		}
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

void Voxel::CalcNormal(VertexBuffer& v1, VertexBuffer& v2, VertexBuffer& v3)
{
	XMFLOAT3 f1 = v1.position - v2.position;
	f1 = Normalize3(f1);
	XMFLOAT3 f2 = v3.position - v2.position;
	f2 = Normalize3(f2);
	XMVECTOR V1 = XMLoadFloat3(&f1);
	XMVECTOR V2 = XMLoadFloat3(&f2);
	XMVECTOR UP = XMVector3Cross(V1, V2);
	UP = XMVector3Normalize(UP);
	XMFLOAT3 n;
	XMStoreFloat3(&n, UP);
	v1.normal = n;
	v2.normal = n;
	v3.normal = n;
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
	return chunksArray[x + y * width + z * width*height];
}

XMFLOAT3 Voxel::CovertToChunkPos(XMFLOAT3 targetPos)
{
	float ol = octree->size * 0.5f;
	return targetPos + XMFLOAT3(ol,ol,ol);
}

void Voxel::UpdateMesh()
{
	if (useGeometry)
	{
		renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_marchingCube"));
	}
	else
		renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_texture"));
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
	if (useMarchingCube)
	{
		GenerateMarchingCubeOctreeFaces();
	}
	else
	{
		if (type == 0)
			GenerateOctreeFaces(octree->root, faceCount);
		else
		{
			std::vector<OctreeNode<int>*> leafs;
			octree->root->GetLeafs(leafs);
			for (auto i : leafs)
			{
				GenerateOctreeFaces2(i, faceCount);
			}
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
	if(vertices.size())
		newMesh->SetVertices(&vertices[0], vertices.size());
	if(indices.size())
		newMesh->SetIndices(&indices[0], indices.size());
	if(!useGeometry)
		newMesh->RecalculateNormals();
	newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
	vertices.clear();
	indices.clear();
	mesh = newMesh;
	renderer->SetMesh(newMesh);
}

void Voxel::LoadHeightMapFromRaw(int _width, int _height,int _depth,const char* filename)
{
	unsigned short** data = nullptr;
	NewChunks(_width, _height,_depth);
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
				SetChunk(x, y, z, 1);
			}
		}
	}
	for (int i = 0; i < width; i++)
		delete[] data[i];
	delete[] data;
	printf("%d byte chunk data 생성 완료\n", width*depth*height);
}

void Voxel::LoadCube(int _width, int _height, int _depth)
{
	NewChunks(_width, _height, _depth);
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			for (int y = 0; y < height; y++)
			{
				SetChunk(x,y,z,1);
			}
		}
	}
}

void Voxel::LoadPerlin(int _width,int _height, int _depth, int _maxHeight, float refinement)
{
	refinement = refinement / 10;
	PerlinNoise perlin;
	if (_maxHeight > _height)
		_maxHeight = _height *0.5f;
	//if (useOctree)
	//{
	//	ULONG tick = GetTickCount64();
	//	NewOctree(_width);
	//	float halfSize = octree->size*0.5f;
	//	for (int x = 0; x < _width; x++)
	//	{
	//		for (int z = 0; z < _width; z++)
	//		{
	//			float noise = perlin.noise((float)x*refinement, (float)z*refinement, 0);
	//			noise *= (float)_maxHeight;
	//			for (int y = 0; y < noise; y++)
	//			{
	//				XMFLOAT3 pos = XMFLOAT3(x - halfSize, y - halfSize, z - halfSize);
	//				octree->root->Insert(pos, 1,0,false);
	//			}
	//		}
	//	}
	//	printf("Build Octree : %dms\n", GetTickCount64() - tick);
	//	return;
	//}
	NewChunks(_width, _height, _depth);
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			float noise = perlin.noise((float)x*refinement, (float)z*refinement, 0);
			noise *= (float)_maxHeight;
			for (int y = 0; y < noise; y++)
			{
				SetChunk(x,y,z,1);
			}
		}
	}
}

void Voxel::SetChunk(int x, int y, int z, BYTE value)
{
	chunksArray[x + y * width + z * width*height]=value;
}

void Voxel::SetOctree(XMFLOAT3 position, BYTE value)
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
	SetChunk(x, y, z, value);
	if (useOctree)
	{
		int lodLevel = GetLODLevel(lastBasePosition, XMFLOAT3(x*unit + 0.1f, y*unit + 0.1f, z*unit + 0.1f));
		if (lodLevel < 0)
			lodLevel = octree->depth;
		octree->root->Insert(XMFLOAT3(x*unit, y*unit, z*unit),GetChunk(x,y,z), lodLevel);
	}
	chunkUpdated = true;
}

void Voxel::BuildOctree(int _octreelength, XMFLOAT3 basePosition)
{
	ULONG tick = GetTickCount64();
	NewOctree(_octreelength);
	int lodLevel = 0;
	float oL = _octreelength * 0.5f;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				if (GetChunk(x, y, z))
				{
					XMFLOAT3 pos = XMFLOAT3(x*unit-oL, y*unit - oL, z*unit - oL);
					lodLevel = GetLODLevel(basePosition, pos);
					if (lodLevel < 0)
						lodLevel = octree->depth;
					octree->root->Insert(pos, GetChunk(x,y,z), lodLevel, octreeMerge);
				}
			}
		}
	}
	printf("Build Octree : %dms\n", GetTickCount64() - tick);
}

void Voxel::NewOctree(int _size)
{
	width = _size;
	height = _size;
	depth = _size;
	if (!octree||octree->size != _size)
	{
		int i = 0;
		int n = _size;
		while ((n = n >> 1))
			++i;
		int depth = i - 1;
		if (octree)
		{
			delete octree;
		}
		octree = new Octree<int>(transform()->GetWorldPosition(), _size, depth, octreeMerge);
	}
	else
	{
		octree->root->RemoveChilds();
	}
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
	return 0;
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
