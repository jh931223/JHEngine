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
#include<atomic>
#include<amp_graphics.h>
#include"SystemClass.h"
#include"MaterialClass.h"
#include"StructuredBuffer.h"
#include"ResourcesClass.h"
#include"InputClass.h"
#include"HierachyClass.h"

using namespace concurrency;
using namespace concurrency::graphics;
using namespace concurrency::direct3d;

Voxel::Voxel()
{
}

Voxel ::~Voxel()
{
	ReleaseChunks();
	ReleaseOctree();
	ReleaseMarchingCube();
	mesh->Shutdown();
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
	if (Input()->GetKey(81))
	{
		XMFLOAT3 pos = camera->transform->GetWorldPosition() + camera->transform->forward() * 3;
		XMFLOAT3 cpos = CovertToChunkPos(pos, false);
		int cX = (int)cpos.x;
		int cY = (int)cpos.y;
		int cZ = (int)cpos.z;
		float radius = 3;
		for (int x = cpos.x - radius; x < cpos.x + radius; x += 1)
		{
			for (int y = cpos.y - radius; y < cpos.y + radius; y += 1)
			{
				for (int z = cpos.z - radius; z < cpos.z + radius; z += 1)
				{
					if (radius >= GetDistance(XMFLOAT3(x + 0.5f, y + 0.5f, z + 0.5f), cpos))
					{
						if (!useOctree)
						{
							SetChunk((int)x, (int)y, (int)z, 0);
						}
						else SetOctree(XMFLOAT3(x, y, z), 0);
					}
				}
			}
		}
		UpdateMesh(false);
	}
	else if (Input()->GetKey(69))
	{
		XMFLOAT3 pos = camera->transform->GetWorldPosition() + camera->transform->forward() * 3;
		XMFLOAT3 cpos = CovertToChunkPos(pos, false);
		int cX = (int)cpos.x;
		int cY = (int)cpos.y;
		int cZ = (int)cpos.z;
		float radius = 3;
		for (int x = cpos.x - radius; x < cpos.x + radius; x+=1)
		{
			for (int y = cpos.y - radius; y < cpos.y + radius; y+=1)
			{
				for (int z = cpos.z - radius; z < cpos.z + radius; z+=1)
				{
					if (radius >= GetDistance(XMFLOAT3(x + 0.5f, y + 0.5f, z + 0.5f), cpos))
					{
						if (!useOctree)
						{
							SetChunk((int)x, (int)y, (int)z, 1);
						}
						else SetOctree(XMFLOAT3(x,y,z), 1);
					}
				}
			}
		}
		UpdateMesh(false);
	}
}

void Voxel::OnStart()
{
	transform()->SetPosition(XMFLOAT3(0, 0, 0));
	Initialize();
}

void Voxel::Initialize()
{
	camera = Hierachy()->FindGameObjectWithName("mainCamera");


	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;
	useMarchingCube = true;
	useOctree = false;
	octreeMerge = false;
	buildWithGPGPU = false;
	useGPGPU = false;
	useGeometry = false;
	
	SetLODLevel(0, 5000000);
	SetLODLevel(1, 100);
	SetLODLevel(2, 200);
	SetLODLevel(3, 300);
	lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	
	octreeType = 0;

	//LoadCube(2, 2, 2);
	LoadPerlin(128, 128, 128, 64, 0.3);
	int h = ReadTXT("../JHEngine/height.txt");




	ULONG tick = GetTickCount64();
	//LoadHeightMapFromRaw(1025,128,1025,"../JHEngine/data/heightmap.r16");
	if(useOctree&&!octree)
		BuildOctree(width, lastBasePosition);
	UpdateMesh();
	printf("Init time : %dms\n", GetTickCount64() - tick);

}




void Voxel::NewChunks(int _width, int _height,int _depth)
{
	if (chunksData)
		ReleaseChunks();
	height = _height;
	width = _width;
	depth = _depth;
	chunksData = new float[width*height*depth]{ 0, };
	memset(chunksData, 0, sizeof(float)*width*height*depth);
	points.clear();
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < depth; k++)
			{
				chunksData[i + j * width + k * width*height] = -1;
				points.push_back(XMFLOAT3(i, j, k));
			}
}
void Voxel::ReleaseChunks()
{
	if (chunksData)
	{
		delete[] chunksData;
	}
	chunksData = 0;
}
void Voxel::ReleaseOctree()
{
	if (octree)
		delete octree;
	octree = 0;
}
void Voxel::ReleaseMarchingCube()
{
	if (mcData)
	{
		delete[] mcData;
	}
	mcData = 0;
	if (mcDataOctree)
	{
		delete mcDataOctree;
	}
	mcDataOctree = 0;
}
void Voxel::CreateFaceUp(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
	v1.uv = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.uv = XMFLOAT2(uv.x, uv.y);
	v3.uv = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.uv = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceDown(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
	v1.uv = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.uv = XMFLOAT2(uv.x, uv.y);
	v3.uv = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.uv = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceRight(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
	v1.uv = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.uv = XMFLOAT2(uv.x, uv.y);
	v3.uv = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.uv = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceLeft(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
	v1.uv = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.uv = XMFLOAT2(uv.x, uv.y);
	v3.uv = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.uv = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);

	faceCount++;
}
void Voxel::CreateFaceForward(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
	v1.uv = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.uv = XMFLOAT2(uv.x, uv.y);
	v3.uv = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.uv = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceBackward(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
	v1.uv = XMFLOAT2(uv.x, uv.y + tUnit);
	v2.uv = XMFLOAT2(uv.x, uv.y);
	v3.uv = XMFLOAT2(uv.x + tUnit, uv.y);
	v4.uv = XMFLOAT2(uv.x + tUnit, uv.y + tUnit);

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void Voxel::CreateFaceMarchingCube(float x, float y, float z, int _unit, BYTE type)
{
	float size = width;
	float size2 = width * height;
	float p = x + width * y + size2 * z;
	float px = p + 1;
	float py = p + size;
	float pxy = py + 1;
	float pz = p + size2;
	float pxz = px + size2;
	float pyz = py + size2;
	float pxyz = pxy + size2;

	float value0 = chunksData[(int)p];
	float value1 = chunksData[(int)px];
	float value2 = chunksData[(int)py];
	float value3 = chunksData[(int)pxy];
	float value4 = chunksData[(int)pz];
	float value5 = chunksData[(int)pxz];
	float value6 = chunksData[(int)pyz];
	float value7 = chunksData[(int)pxyz];

	int cubeindex = 0;

	if (value0 < isoLevel) cubeindex |= 1;
	if (value1 < isoLevel) cubeindex |= 2;
	if (value2 < isoLevel) cubeindex |= 8;
	if (value3 < isoLevel) cubeindex |= 4;
	if (value4 < isoLevel) cubeindex |= 16;
	if (value5 < isoLevel) cubeindex |= 32;
	if (value6 < isoLevel) cubeindex |= 128;
	if (value7 < isoLevel) cubeindex |= 64;

	XMFLOAT3 vert[12] = { XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0) };
	XMFLOAT2 uv = GetUV(type);
	int bits=edgeTable[cubeindex];
	if (bits == 0)
		return;
	float mu = 0.5f;
	if ((bits & 1) != 0)
	{
		mu = (isoLevel - value0) / (value1 - value0);
		vert[0] = lerpSelf(points[(int)p], points[(int)px], mu);


	}
	if ((bits & 2) != 0)
	{
		mu = (isoLevel - value1) / (value3 - value1);
		vert[1] = lerpSelf(points[(int)px], points[(int)pxy], mu);
	}
	if ((bits & 4) != 0)
	{
		mu = (isoLevel - value2) / (value3 - value2);
		vert[2] = lerpSelf(points[(int)py], points[(int)pxy], mu);

	}
	if ((bits & 8) != 0)
	{
		mu = (isoLevel - value0) / (value2 - value0);
		vert[3] = lerpSelf(points[(int)p], points[(int)py], mu);

	}
	// top of the cube
	if ((bits & 16) != 0)
	{
		mu = (isoLevel - value4) / (value5 - value4);
		vert[4] = lerpSelf(points[(int)pz], points[(int)pxz], mu);

	}
	if ((bits & 32) != 0)
	{
		mu = (isoLevel - value5) / (value7 - value5);
		vert[5] = lerpSelf(points[(int)pxz], points[(int)pxyz], mu);

	}
	if ((bits & 64) != 0)
	{
		mu = (isoLevel - value6) / (value7 - value6);
		vert[6] = lerpSelf(points[(int)pyz], points[(int)pxyz], mu);
	}
	if ((bits & 128) != 0)
	{
		mu = (isoLevel - value4) / (value6 - value4);
		vert[7] = lerpSelf(points[(int)pz], points[(int)pyz], mu);

	}
	// vertical lines of the cube
	if ((bits & 256) != 0)
	{
		mu = (isoLevel - value0) / (value4 - value0);
		vert[8] = lerpSelf(points[(int)p], points[(int)pz], mu);

	}
	if ((bits & 512) != 0)
	{
		mu = (isoLevel - value1) / (value5 - value1);
		vert[9] = lerpSelf(points[(int)px], points[(int)pxz], mu);

	}
	if ((bits & 1024) != 0)
	{
		mu = (isoLevel - value3) / (value7 - value3);
		vert[10] = lerpSelf(points[(int)pxy], points[(int)pxyz], mu);
	}

	//print (bits & 2048);

	if ((bits & 2048) != 0)
	{
		mu = (isoLevel - value2) / (value6 - value2);
		vert[11] = lerpSelf(points[(int)py], points[(int)pyz], mu);
	}
	//cubeindex <<= 4;  // multiply by 16...
	int i = 0;
	int rr = 0;
	while (triTable[cubeindex][i] != -1)
	{
		int index1 = triTable[cubeindex][i];
		int index2 = triTable[cubeindex][i+1];
		int index3 = triTable[cubeindex][i + 2];
		VertexBuffer vert1;
		vert1.position = vert[index1];
		VertexBuffer vert2;
		vert2.position = vert[index2];
		VertexBuffer vert3;
		vert3.position = vert[index3];
		rr = 1 - rr; // temp variable.. every 2nd UV set is different order..

		if (rr == 0)
		{
			vert1.uv = XMFLOAT2(0, 0);
			vert2.uv = XMFLOAT2(0, 1);
			vert3.uv = XMFLOAT2(1, 1);
		}
		else
		{
			vert1.uv = XMFLOAT2(1, 0);
			vert2.uv = XMFLOAT2(0,0);
			vert3.uv = XMFLOAT2(1, 1);
		}
		int vertexIndex = vertices.size();
		vertices.push_back(vert3);
		indices.push_back(vertexIndex);
		vertices.push_back(vert2);
		indices.push_back(vertexIndex + 1);
		vertices.push_back(vert1);
		indices.push_back(vertexIndex + 2);
		i += 3;
	}
}
void Voxel::AddMarchingCase(int x, int y, int z, int _index)
{
	int w = width + 1, h = height + 1, d = depth + 1;
	if (!mcData)
	{
		mcData = new float[w*h*d];
		memset(mcData, 0, sizeof(unsigned int)*w*h*d);
		//vertices.resize(w*h*d);
	}
	int idx = (x)+(y)* w + (z)* h * d;
	if (mcData[idx] == 0&&useGeometry)
	{
		VertexBuffer vert;
		vert.position = XMFLOAT3(x, y, z);// - XMFLOAT3((w - 1)*0.5f, (h - 1)*0.5f, (d - 1)*0.5f);
		um_Vertices[idx] = vert;
		//vertices[idx] = vert;
	}
	else if (mcData[idx] == 255)
		return;
	//mcData[idx] |= _index;
	vertices[idx].uv.x = mcData[idx];
	if (mcData[idx] == 255)
		um_Vertices.erase(idx);
}
void Voxel::SubMarchingCase(int x, int y, int z, int _index)
{
	if (!mcData)
		return;
	int w = width + 1, h = height + 1, d = depth + 1;
	int idx = (x)+(y)* w + (z)* h * d;
	if (mcData[idx] == 0)
		return;
	bool hasNoVertex = mcData[idx] == 255;
	//mcData[idx] &= ~(_index);
	//vertices[idx].uv.x = mcData[idx];
	if (useGeometry)
	{
		if (mcData[idx] == 0)
		{
			um_Vertices.erase(idx);
			return;
		}
		else if (hasNoVertex)
		{
			VertexBuffer vert;
			vert.position = XMFLOAT3(x, y, z);// -XMFLOAT3((w - 1)*0.5f, (h - 1)*0.5f, (d - 1)*0.5f);
			um_Vertices[idx] = vert;
		}
	}
}
void Voxel::SetMarchingCubeChunkData(int x, int y, int z,bool isCreate,int length)
{
	if(length>1)
	{
		int size[3] = { width + 1,height + 1,depth + 1 };
		int i, j, k;
		int maxX = x + length;
		if (maxX > width)
			maxX = width;
		int maxY = y + length;
		if (maxY > height)
			maxY = height;
		int maxZ = z + length;
		if (maxZ > depth)
			maxZ = depth;
		for (i=x; i < maxX; i++)
		{
			for (j=y; j < maxY; j++)
			{
				for (k=z; k < maxZ; k++)
				{
					if (i > x&&i < maxX - 1)
					{
						if (j > y&&j < maxY - 1)
						{
							if (k > z&&k < maxZ - 1)
							{
								int idx = i + j * size[0] + k * size[0] * size[1];
								mcData[idx] = 255;
								mcData[idx + size[0] * size[1]] = 255;
								mcData[idx + size[0]] = 255;
								mcData[idx + size[0] + size[0] * size[1]] = 255;

								mcData[idx + 1] = 255;
								mcData[idx + 1 + size[0] * size[1]] = 255;
								mcData[idx + 1 + size[0] + size[0] * size[1]] = 255;
								mcData[idx + 1 + size[0]] = 255;
								continue;
							}
						}
					}
					SetMarchingCubeChunkData(i, j, k, isCreate);
				}
			}
		}
	}
	else
	{
		if (x >= width || x < 0 || y >= height || y < 0 || z >= depth || z < 0)
			return;
		int size[3] = { width + 1,height + 1,depth + 1 };
		if (isCreate)
		{
			AddMarchingCase(x, y, z, 32);//
			AddMarchingCase(x + 1, y, z, 16);//
			AddMarchingCase(x, y + 1, z, 2);
			AddMarchingCase(x + 1, y + 1, z, 1);
			AddMarchingCase(x, y, z + 1, 64);//
			AddMarchingCase(x + 1, y, z + 1, 128);//
			AddMarchingCase(x, y + 1, z + 1, 4);//
			AddMarchingCase(x + 1, y + 1, z + 1, 8);//
		}
		else
		{
			SubMarchingCase(x, y, z, 32);
			SubMarchingCase(x + 1, y, z, 16);
			SubMarchingCase(x, y + 1, z, 2);
			SubMarchingCase(x + 1, y + 1, z, 1);
			SubMarchingCase(x, y, z + 1, 64);
			SubMarchingCase(x + 1, y, z + 1, 128);
			SubMarchingCase(x, y + 1, z + 1, 4);
			SubMarchingCase(x + 1, y + 1, z + 1, 8);
		}
	}
}
void Voxel::SetupMarchingCubeVertexBufferGS()
{
	if (useGeometry)
	{
		vertices.clear();
		for (auto i : um_Vertices)
		{
			i.second.uv.x = mcData[i.first];
			vertices.push_back(i.second);
		}
	}
}

//螟お葬 ィ葬堆 儅撩

void Voxel::CreateOctreeFaces(OctreeNode<float>* current,int& faceCount)
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
				CreateOctreeFaces(current->GetChild(i), faceCount);
			}
		}
	}
}
void Voxel::CreateOctreeFaces2(OctreeNode<float>* node, int& faceCount)
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

//葆蘆聽粽 ィ葬堆 儅撩

void Voxel::GenerateMarchingCubeFaces(bool isNew)
{
	ULONG time = GetTickCount();
	for (int x = 0; x < width-1; x++)
	{
		for (int y = 0; y < height-1; y++)
		{
			for (int z = 0; z < depth-1; z++)
			{
				CreateFaceMarchingCube(x, y, z, unit, -1);
			}
		}
	}
	printf("Marching Cube Create Buffer ( CPU ): %d ms\n", GetTickCount() - time);
	
}
void Voxel::GenerateMarchingCubeFaces_GS(bool isNew)
{
	int size[3]{ width + 1,height + 1,depth + 1 };
	ULONG time = GetTickCount();
	if (isNew)
	{
		ULONG time2 = GetTickCount();
		if (useOctree)
		{
			std::vector<OctreeNode<float>*> leafs;
			octree->root->GetLeafs(leafs);
			for (auto i : leafs)
			{
				if (i->GetValue())
				{
					int x = i->GetStartPosition().x;
					int y = i->GetStartPosition().y;
					int z = i->GetStartPosition().z;
					SetMarchingCubeChunkData(x, y, z, true,(int)i->GetCellSize());
				}
			}
		}
		else
		{
			for (int x = 0; x < width; x++)
			{
				for (int y = 0; y < height; y++)
				{
					for (int z = 0; z < depth; z++)
					{
						if (GetChunk(x, y, z))
						{
							SetMarchingCubeChunkData(x, y, z, true);
						}
					}
				}
			}
		}
		printf("Marching Cube Build Data ( Geometry, useOctree=%d ): %d ms\n", useOctree, GetTickCount() - time2);
	}
	int nums = size[0] * size[1] * size[2];
	Material* material = renderer->GetMaterial();
	ULONG time3 = GetTickCount();
	//SetupMarchingCubeVertexBufferGS();
	printf("Marching Cube Build Vertex Buffer ( Geometry ): %d ms\n", GetTickCount() - time3);
	material->GetParams()->SetInt("vertCount", vertices.size());
	material->GetParams()->SetFloat3("startPosition", vertices[0].position);
	material->GetParams()->SetInt("length", size[0]);
	/*ID3D11Device* device = SystemClass::GetInstance()->GetDevice();
	material->GetParams()->SetSRV("mcData", new StructuredBuffer(device,sizeof(int),nums,mcData));*/
	printf("Marching Cube Create Buffer ( Geometry ): %d ms\n", GetTickCount() - time);
}
void Voxel::GenerateMarchingCubeFaces_GPGPU(bool isNew)
{
	ULONG time = GetTickCount();
	/*int size[3]{ width + 1,height + 1,depth + 1 };
	const size_t SIZE = size[0] * size[1] * size[2] * 3 * 5;

	array_view<const unsigned int, 3> cubes(size[0], size[1], size[2], mcData);


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
	parallel_for_each(cubes.extent, [=](index<3> idxs)restrict(amp)
	{
		unsigned int _case = cubes[idxs[0]][idxs[1]][idxs[2]];
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
	for (int i = 0; i < SIZE; i++)
	{
		if (gVertBuffer[i].w == 1)
		{
			VertexBuffer vert;
			vert.position = XMFLOAT3(gVertBuffer[i].x, gVertBuffer[i].y, gVertBuffer[i].z);
			vert.uv = XMFLOAT2(gTexBuffer[i].x, gTexBuffer[i].y);
			indices.push_back(vertices.size());
			vertices.push_back(vert);
		}
	}*/
	printf("Marching Cube Create Buffer ( GPU ): %d ms\n", GetTickCount() - time);
}
void Voxel::GenerateMarchingCubeFaces_Octree(bool isNew)
{

	if (isNew)
	{
		delete mcDataOctree;
		mcDataOctree = new Octree<short>(transform()->GetWorldPosition(), octree->size * 2, octree->depth + 1);
		std::vector<OctreeNode<float>*> leafs;
		octree->root->GetLeafs(leafs);
		for (auto i : leafs)
		{
			if (i->GetValue())
			{
				int x = i->GetPosition().x;
				int y = i->GetPosition().y;
				int z = i->GetPosition().z;

				OctreeNode<short>* n;
				n = OctreeNode<short>::Subdivide(mcDataOctree->root, XMFLOAT3(x, y, z), mcDataOctree->depth);
				n->SetValue(n->GetValue() | 0b00100000);
				n = OctreeNode<short>::Subdivide(mcDataOctree->root, XMFLOAT3(x + 1, y, z), mcDataOctree->depth);
				n->SetValue(n->GetValue() | 0b00010000);
				n = OctreeNode<short>::Subdivide(mcDataOctree->root, XMFLOAT3(x, y + 1, z), mcDataOctree->depth);
				n->SetValue(n->GetValue() | 0b00000010);
				n = OctreeNode<short>::Subdivide(mcDataOctree->root, XMFLOAT3(x + 1, y + 1, z), mcDataOctree->depth);
				n->SetValue(n->GetValue() | 0b00000001);
				n = OctreeNode<short>::Subdivide(mcDataOctree->root, XMFLOAT3(x, y, z + 1), mcDataOctree->depth);
				n->SetValue(n->GetValue() | 0b01000000);
				n = OctreeNode<short>::Subdivide(mcDataOctree->root, XMFLOAT3(x + 1, y, z + 1), mcDataOctree->depth);
				n->SetValue(n->GetValue() | 0b10000000);
				n = OctreeNode<short>::Subdivide(mcDataOctree->root, XMFLOAT3(x, y + 1, z + 1), mcDataOctree->depth);
				n->SetValue(n->GetValue() | 0b00000100);
				n = OctreeNode<short>::Subdivide(mcDataOctree->root, XMFLOAT3(x + 1, y + 1, z + 1), mcDataOctree->depth);
				n->SetValue(n->GetValue() | 0b00001000);
			}
		}
	}

	

	std::vector<OctreeNode<short>*> cleafs;
	mcDataOctree->root->GetLeafs(cleafs);
	for (auto i : cleafs)
	{
		CreateFaceMarchingCube(i->GetPosition().x , i->GetPosition().y , i->GetPosition().z , unit, 1);
	}
	delete mcDataOctree;

}

void Voxel::GenerateMarchingCubeFaces_GPGPU_GS(bool isNew)
{
	int size[3]{ width + 1,height + 1,depth + 1 };
	ULONG time = GetTickCount();
	if (isNew)
	{
		ULONG time2 = GetTickCount();
		//////////////////////////////////////////////////////////////////
		//if (mcData)
		//	delete mcData;
		//mcData = new float[size[0] * size[1] * size[2]];
		////memset(mcData, 0, size[0] * size[1] * size[2]);
		//ULONG time3 = GetTickCount();
		//array<unsigned int, 3> mcData_array(size[0],size[1],size[2],mcData);
		//extent<3> ex(size[0], size[1], size[2]);
		//array_view<unsigned int, 3> chunksData_view(width, height, depth, chunksData);
		//parallel_for_each(mcData_array.extent.tile<2,2,2>(), [&,chunksData_view](tiled_index<2,2,2> idxs)restrict(amp)
		//{
		//	index<3> idx = idxs.global;
		//	index<3> leftBottomBackward = idx + index<3>(-1, -1, -1);//32
		//	index<3> leftBottomForward = idx + index<3>(-1, -1, 0);
		//	index<3> leftUpBackward = idx + index<3>(-1, 1, -1);
		//	index<3> leftUpForward = idx + index<3>(-1, 0, 0);
		//	index<3> rightUpBackward = idx + index<3>(0, 0, -1);
		//	index<3> rightUpForward = idx + index<3>(0, 0, 0);//
		//	index<3> rightDownBackward = idx + index<3>(1, -1, -1);
		//	index<3> rightDownForward = idx + index<3>(1, -1, 0);
		//	
		//	/*
		//	    券天天天天天天天天天天天天刻
		//	   /l                         /l
		//	  / l                        / l
		//	 /  l                       /  l
		//	/   l                      /   l
		//   /    l                     /    l
		//  /     l                    /     l
		// 函天天天天天天天天天天天天佾      l
		// l      l                   I      l
		// l      刮__________________l_____到
		// l     /                    l     /
		// l    /                     l    /
		// l   /                      l   /
		// l  /                       l  /
		// l /                        l /
		// l/                         l/
		// 刺天天天天天天天天天天天天刷
		//	
		//	
		//	
		//	
		//	*/

		//	if (rightUpBackward[0] >= 0 && rightUpBackward[1] >= 0 && rightUpBackward[2] >= 0)
		//	{
		//		if(rightUpBackward[0] < chunksData_view.extent[0] && rightUpBackward[1] < chunksData_view.extent[1] && rightUpBackward[2] < chunksData_view.extent[2])
		//			if(chunksData_view[rightUpBackward])
		//				mcData_array[idx] |= 1;
		//	}
		//	if (rightUpForward[0]>=0 && rightUpForward[1]>=0 && rightUpForward[2]>=0)
		//	{
		//		if (rightUpForward[0] < chunksData_view.extent[0] && rightUpForward[1] < chunksData_view.extent[1] && rightUpForward[2]< chunksData_view.extent[2])
		//			if(chunksData_view[rightUpForward])
		//				mcData_array[idx] |= 4;
		//	}
		//	if (leftUpBackward[0] >= 0 && leftUpBackward[1] >= 0 && leftUpBackward[2]>=0)
		//	{
		//		if (leftUpBackward[0] < chunksData_view.extent[0] && leftUpBackward[1] < chunksData_view.extent[1] && leftUpBackward[2]< chunksData_view.extent[2])
		//			if (chunksData_view[leftUpBackward])
		//				mcData_array[idx] |= 2;
		//	}
		//	if (leftUpForward[0] >= 0 && leftUpForward[1] >= 0 && leftUpForward[2] >= 0)
		//	{
		//		if (leftUpForward[0] < chunksData_view.extent[0] && leftUpForward[1] < chunksData_view.extent[1] && leftUpForward[2]< chunksData_view.extent[2])
		//			if (chunksData_view[leftUpForward])
		//				mcData_array[idx] |= 8;
		//	}
		//	if (rightDownBackward[0] >= 0 && rightDownBackward[1] >= 0 && rightDownBackward[2] >= 0)
		//	{
		//		if (rightDownBackward[0] < chunksData_view.extent[0] && rightDownBackward[1] < chunksData_view.extent[1] && rightDownBackward[2]< chunksData_view.extent[2])
		//			if (chunksData_view[rightDownBackward])
		//				mcData_array[idx] |= 16;
		//	}
		//	if (leftBottomBackward[0] >= 0 && leftBottomBackward[1] >= 0 && leftBottomBackward[2] >= 0)
		//	{
		//		if (leftBottomBackward[0] < chunksData_view.extent[0] && leftBottomBackward[1] < chunksData_view.extent[1] && leftBottomBackward[2]< chunksData_view.extent[2])
		//			if (chunksData_view[leftBottomBackward])
		//				mcData_array[idx] |= 32;
		//	}
		//	if (rightDownForward[0] >= 0 && rightDownForward[1] >= 0 && rightDownForward[2] >= 0)
		//	{
		//		if (rightDownForward[0] < chunksData_view.extent[0] && rightDownForward[1] < chunksData_view.extent[1] && rightDownForward[2]< chunksData_view.extent[2])
		//			if (chunksData_view[rightDownForward])
		//				mcData_array[idx] |= 64;
		//	}
		//	if (leftBottomForward[0] >= 0 && leftBottomForward[1] >= 0 && leftBottomForward[2] >= 0)
		//	{
		//		if (leftBottomForward[0] < chunksData_view.extent[0] && leftBottomForward[1] < chunksData_view.extent[1] && leftBottomForward[2]< chunksData_view.extent[2])
		//			if (chunksData_view[leftBottomForward])
		//				mcData_array[idx] |= 128;
		//	}



		//	//if (chunksData_view[idx])
		//	//{
		//	//	atomic_fetch_or(&(mcData_array[idx]), 32);
		//	//	atomic_fetch_or(&(mcData_array[idx+ index<3>(1, 0, 0)]), 16);
		//	//	atomic_fetch_or(&(mcData_array[idx + index<3>(0, 1, 0)]), 2);
		//	//	atomic_fetch_or(&(mcData_array[idx + index<3>(1, 1, 0)]), 1);
		//	//	atomic_fetch_or(&(mcData_array[idx + index<3>(0, 0, 1)]),64);
		//	//	atomic_fetch_or(&mcData_array[idx + index<3>(1, 0, 1)],128);
		//	//	atomic_fetch_or(&mcData_array[idx + index<3>(0, 1, 1)],4);
		//	//	atomic_fetch_or(&mcData_array[idx + index<3>(1, 1, 1)],8);
		//	//	
		//	//}
		//});
		//printf("Marching Cube Build Data GPGPU Processing ( Geometry, buildWithGPGPU=%d ): %d ms\n", buildWithGPGPU, GetTickCount() - time3);
		//ULONG time4 = GetTickCount();
		//std::vector<unsigned int> mcDataVector(size[0]*size[1]*size[2]);
		//copy(mcData_array, mcDataVector.begin());
		//printf("Marching Cube Build Data Copy To CPU ( Geometry, buildWithGPGPU=%d ): %d ms\n", buildWithGPGPU, GetTickCount() - time4);
		//for (int x = 0; x < size[0]; x++)
		//{
		//	for (int y = 0; y < size[1]; y++)
		//	{
		//		for (int z = 0; z < size[2]; z++)
		//		{
		//			int idx = x + y * size[0] + z * size[0] * size[1];
		//			if (mcDataVector[idx]&&mcDataVector[idx]!=255)
		//			{
		//				VertexBuffer vert;
		//				vert.position = XMFLOAT3(x + 0.5f, y + 0.5f, z + 0.5f);
		//				vert.uv.x = mcDataVector[idx];
		//				vertices.push_back(vert);
		//				//um_Vertices.insert[x+y*size[0]+z*size[0]*size[1]]=(vert);
		//			}
		//		}
		//	}
		//}
		printf("Marching Cube Build Data ( Geometry, buildWithGPGPU=%d ): %d ms\n", buildWithGPGPU, GetTickCount() - time2);
	}
	int nums = size[0] * size[1] * size[2];
	Material* material = renderer->GetMaterial();
	ULONG time3 = GetTickCount();
	//SetupMarchingCubeVertexBufferGS();
	printf("Marching Cube Build Vertex Buffer ( Geometry ): %d ms\n", GetTickCount() - time3);
	material->GetParams()->SetInt("vertCount", vertices.size());
	material->GetParams()->SetFloat3("startPosition", vertices[0].position);
	material->GetParams()->SetInt("length", size[0]);
	/*ID3D11Device* device = SystemClass::GetInstance()->GetDevice();
	material->GetParams()->SetSRV("mcData", new StructuredBuffer(device,sizeof(int),nums,mcData));*/
	printf("Marching Cube Create Buffer ( Geometry ): %d ms\n", GetTickCount() - time);
}
void Voxel::GenerateMarchingCubeFacesOctreeVer2()
{
	int size[3] = { width + 1, height + 1, depth + 1 };
	if (mcData)
		delete[] mcData;
	int nums = size[0] * size[1] * size[2];
	mcData=new float[nums];
	//memset(mcData, 0, sizeof(unsigned int)*nums);


	std::vector<OctreeNode<float>*> leafs;
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
			    券天天天天天天天天天天天天刻
			   /l                         /l
			  / l                        / l
			 /  l                       /  l
			/   l                      /   l
		   /    l                     /    l
		  /     l                    /     l
		 函天天天天天天天天天天天天佾      l
		 l      l                   I      l
		 l      刮__________________l_____到
		 l     /                    l     /
		 l    /                     l    /
		 l   /                      l   /
		 l  /                       l  /
		 l /                        l /
		 l/                         l/
		 刺天天天天天天天天天天天天刷
			
			
			
			
			*/

			SetMarchingCubeChunkData(x, y, z, true);
		}
	}
	for (int x = 0; x < size[0]; x++)
	{
		for (int y = 0; y <size[1]; y++)
		{
			for (int z = 0; z < size[2]; z++)
			{
				CreateFaceMarchingCube(x, y, z, unit, -1);
			}
		}
	}
}

//ィ葬堆 儅撩

void Voxel::GenerateVoxelFaces()
{
	int faceCount = 0;
	float myBlock;
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

void Voxel::GenerateOctreeFaces(int type)
{
	int faceCount = 0;
	if (type == 0)
		CreateOctreeFaces(octree->root, faceCount);
	else
	{
		std::vector<OctreeNode<float>*> leafs;
		octree->root->GetLeafs(leafs);
		for (auto i : leafs)
		{
			CreateOctreeFaces2(i, faceCount);
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


XMFLOAT2 Voxel::GetUV(BYTE type)
{
	if (type == -1)
		return XMFLOAT2(0, 0);
	return XMFLOAT2(tUnit*(type - 1), (int)(type / tAmount) * tUnit);
}

float Voxel::GetChunk(int x, int y, int z)
{
	if (x >= width || y >= height || z >= depth)
		return 0;
	if (x < 0 || y < 0 || z < 0)
		return 0;
	return chunksData[x + y * width + z * width*height];
}

void Voxel::SetChunk(int x, int y, int z, float value)
{
	if (x >= width || y >= height || z >= depth)
		return;
	if (x < 0 || y < 0 || z < 0)
		return;
	if(chunksData)
		chunksData[x + y * width + z * width*height]=value;
	if (useMarchingCube&&mcData)
	{
		SetMarchingCubeChunkData(x, y, z, (value>0));
	}
}
XMFLOAT3 Voxel::CovertToChunkPos(XMFLOAT3 targetPos, bool returnNan)
{
	XMFLOAT3 newpos = transform()->GetWorldPosition();
	XMFLOAT3 startPos = newpos;// - XMFLOAT3(width*0.5f, width*0.5f, width*0.5f);
	XMFLOAT3 endPos = newpos + XMFLOAT3(width, height, depth);
	if (returnNan)
	{
		if (targetPos.x<startPos.x || targetPos.y<startPos.y || targetPos.z<startPos.z || targetPos.x>endPos.x || targetPos.y>endPos.y || targetPos.z>endPos.z)
		{
			return GetNanVector();
		}
	}
	newpos = targetPos - startPos;
	return XMFLOAT3((int)newpos.x, (int)newpos.y, (int)newpos.z);
}

void Voxel::UpdateMesh(bool isNew)
{
	if (useGeometry&&useMarchingCube)
	{
		renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_marchingCube"));
	}
	else
		renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_texture"));
	ULONG tick = GetTickCount64();

	if (!useMarchingCube)
	{
		UpdateVoxelMesh();
	}
	else
	{
		UpdateMarchingCubeMesh(isNew);
	}
	printf("Update Mesh : %dms\n", GetTickCount64() - tick);
}

void Voxel::UpdateVoxelMesh()
{
	if (!useOctree)
		GenerateVoxelFaces();
	else
		GenerateOctreeFaces(1);

	if (mesh)
	{
		mesh->ShutdownBuffers();
		delete mesh;
		mesh = NULL;
	}
	Mesh* newMesh = new Mesh();
	if (vertices.size())
		newMesh->SetVertices(&vertices[0], vertices.size());
	if (indices.size())
		newMesh->SetIndices(&indices[0], indices.size());
	if (!useGeometry)
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

void Voxel::UpdateMarchingCubeMesh(bool isNew)
{
	if (useMarchingCube)
	{
		if (buildWithGPGPU && !useOctree)
			GenerateMarchingCubeFaces_GPGPU_GS(isNew);
		else if (useGPGPU)
			GenerateMarchingCubeFaces_GPGPU(isNew);
		else if (useOctree && !useGeometry)
			GenerateMarchingCubeFaces_Octree(isNew);
		else if(useGeometry)
			GenerateMarchingCubeFaces_GS(isNew);
		else
			GenerateMarchingCubeFaces(isNew);
	}
	else
		return;
	if (mesh)
	{
		mesh->ShutdownBuffers();
		delete mesh;
		mesh = NULL;
	}
	Mesh* newMesh = new Mesh();
	if(vertices.size())
		newMesh->SetVertices(&vertices[0], vertices.size());
	if(indices.size()&& !useGeometry)
		newMesh->SetIndices(&indices[0], indices.size());
	if(!useGeometry)
		newMesh->RecalculateNormals();
	newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
	if (!useGeometry)
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
	printf("%d BYTE chunk data 儅撩 諫猿\n", width*depth*height);
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
	ULONG tick = GetTickCount64();
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
				chunksData[x+y*width+z*width*height] = noise-y;
				//SetChunk(x,y,z,1);
			}
		}
	}
	printf("Load Perlin time : %dms\n", GetTickCount64() - tick);
}


void Voxel::SetOctree(XMFLOAT3 position, BYTE value)
{
	XMFLOAT3 cPos=CovertToChunkPos(position);
	int x = cPos.x, y = cPos.y, z = cPos.z;
	if (x >= width || y >= height || z >= depth)
		return;
	if (x < 0 || y < 0 || z < 0)
		return;
	int lodLevel = GetLODLevel(lastBasePosition, position);
	if (lodLevel < 0)
		lodLevel = octree->depth;
	octree->root->Insert(position,value, lodLevel);
	chunkUpdated = true;
	if (useMarchingCube&&mcData)
	{
		SetMarchingCubeChunkData(x, y, z, (value>0));
	}
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
					XMFLOAT3 pos = transform()->GetWorldPosition()+ XMFLOAT3(x*unit, y*unit, z*unit);// XMFLOAT3(x*unit - oL, y*unit - oL, z*unit - oL);
					lodLevel = GetLODLevel(basePosition, pos);
					if (lodLevel < 0)
						lodLevel = octree->depth;
					octree->root->Insert(pos, GetChunk(x,y,z), lodLevel);
				}
			}
		}
	}
	ReleaseChunks();
	printf("Build Octree : %dms\n", GetTickCount64() - tick);
}

void Voxel::NewOctree(int _size)
{
	width = _size;
	//height = _size;
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
		octree = new Octree<float>(transform()->GetWorldPosition()+XMFLOAT3(width*0.5f, width*0.5f, width*0.5f), _size, depth);
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
