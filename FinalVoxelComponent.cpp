#pragma once
#include"stdafx.h"
#include"FinalVoxelComponent.h"
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

VoxelComponent::VoxelComponent()
{
}

VoxelComponent ::~VoxelComponent()
{
	ReleaseChunks();
	ReleaseOctree();
	mesh->Shutdown();
}

void VoxelComponent::Update()
{
	if (Input()->GetKey(DIK_UP))
	{
		strength += 0.1f;
	}
	else if (Input()->GetKey(DIK_DOWN))
	{
		strength = (strength <= 0) ? 0 : strength-0.1f;
	}

	if (Input()->GetKey(DIK_RIGHT))
	{
		brushRadius += 0.01f;
	}
	else if (Input()->GetKey(DIK_LEFT))
	{
		brushRadius = (brushRadius <= 0) ? 0 : brushRadius - 0.01f;
	}
	if (Input()->GetKey(VK_RBUTTON))
	{
		XMFLOAT3 pos = camera->transform->GetWorldPosition() + camera->transform->forward() * brushRadius;
		XMFLOAT3 cpos = CovertToChunkPos(pos, false);
		int cX = (int)cpos.x;
		int cY = (int)cpos.y;
		int cZ = (int)cpos.z;
		float radius = brushRadius;
		for (int x = cpos.x - radius; x < cpos.x + radius; x += 1)
		{
			for (int y = cpos.y - radius; y < cpos.y + radius; y += 1)
			{
				for (int z = cpos.z - radius; z < cpos.z + radius; z += 1)
				{
					float dis = GetDistance(XMFLOAT3(x + 0.5f, y + 0.5f, z + 0.5f), cpos);
					if (radius >= dis)
					{
						float amount = 1 - dis / radius;
						VoxelData vox=GetChunk(x,y,z);
						vox.isoValue -= strength*amount;
						if (!useMarchingCube)
						{
							vox.material = 0;
						}
						SetChunk((int)x, (int)y, (int)z, vox);
					}
				}
			}
		}
		UpdateMesh(false);
	}
	else if (Input()->GetKey(VK_LBUTTON))
	{
		XMFLOAT3 pos = camera->transform->GetWorldPosition() + camera->transform->forward() *brushRadius;
		XMFLOAT3 cpos = CovertToChunkPos(pos, false);
		int cX = (int)cpos.x;
		int cY = (int)cpos.y;
		int cZ = (int)cpos.z;
		float radius = brushRadius;
		for (int x = cpos.x - radius; x < cpos.x + radius; x+=1)
		{
			for (int y = cpos.y - radius; y < cpos.y + radius; y+=1)
			{
				for (int z = cpos.z - radius; z < cpos.z + radius; z+=1)
				{
					float dis = GetDistance(XMFLOAT3(x + 0.5f, y + 0.5f, z + 0.5f), cpos);
					if (radius >= dis)
					{
						float amount = 1 - dis / radius;
						VoxelData vox = GetChunk(x, y, z);
						vox.isoValue += strength * amount;
						if (!useMarchingCube)
						{
							vox.material = 1;
						}
						SetChunk((int)x, (int)y, (int)z, vox);
					}
				}
			}
		}
		UpdateMesh(false);
	}
}

void VoxelComponent::OnStart()
{
	transform()->SetPosition(XMFLOAT3(0, 0, 0));
	Initialize();
}

void VoxelComponent::Initialize()
{
	camera = Hierachy()->FindGameObjectWithName("mainCamera");


	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;

	useMarchingCube = true;
	useOctree = true;
	useGPGPU = false;
	useGeometry = true;
	
	SetLODLevel(0, 5000000);
	SetLODLevel(1, 100);
	SetLODLevel(2, 200);
	SetLODLevel(3, 300);
	lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	

	//LoadCube(2, 2, 2);
	LoadPerlin(128, 128, 128, 64, 0.3);
	//int h = ReadTXT("/data/height.txt");




	ULONG tick = GetTickCount64();
	//LoadHeightMapFromRaw(1025,128,1025,"../JHEngine/data/heightmap.r16");
	if(useOctree&&!aOctree)
		BuildOctree(width, lastBasePosition);
	UpdateMesh();
	printf("Init time : %dms\n", GetTickCount64() - tick);

}




void VoxelComponent::NewChunks(int _width, int _height,int _depth)
{
	if (chunksData)
		ReleaseChunks();
	height = _height;
	width = _width;
	depth = _depth;
	chunksData = new VoxelData[width*height*depth];
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < depth; k++)
			{
				chunksData[i + j * width + k * width*height].isoValue=-1;
			}
}
void VoxelComponent::ReleaseChunks()
{
	if (chunksData)
	{
		delete[] chunksData;
	}
	chunksData = 0;
}
void VoxelComponent::ReleaseOctree()
{
	if (aOctree)
		delete aOctree;
	aOctree = 0;
}
void VoxelComponent::CreateFaceUp(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
void VoxelComponent::CreateFaceDown(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
void VoxelComponent::CreateFaceRight(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
void VoxelComponent::CreateFaceLeft(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
void VoxelComponent::CreateFaceForward(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
void VoxelComponent::CreateFaceBackward(float x, float y, float z, float _unit, BYTE type, int& faceCount)
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
void VoxelComponent::CreateFaceMarchingCube(float x, float y, float z, int _unit)
{
	float size = width;
	float size2 = width * height;
	float p = x + width * y + size2 * z;
	XMFLOAT3 p3(x, y, z);
	int px = p + 1;
	XMFLOAT3 px3(x + 1, y, z);
	int py = p + size;
	XMFLOAT3 py3(x, y + 1, z);
	int pxy = py + 1;
	XMFLOAT3 pxy3(x + 1, y + 1, z);
	int pz = p + size2;
	XMFLOAT3 pz3(x, y, z + 1);
	int pxz = px + size2;
	XMFLOAT3 pxz3(x + 1, y, z + 1);
	int pyz = py + size2;
	XMFLOAT3 pyz3(x, y + 1, z + 1);
	int pxyz = pxy + size2;
	XMFLOAT3 pxyz3(x + 1, y + 1, z + 1);
	float value0, value1, value2,  value3,  value4,  value5, value6, value7;
	short material = 0;
	if (useOctree)
	{
		VoxelData pData = aOctree->GetValue(p);
		value0 = pData.isoValue;
		value1 = aOctree->GetValue(px).isoValue;
		value2 = aOctree->GetValue(py).isoValue;
		value3 = aOctree->GetValue(pxy).isoValue;
		value4 = aOctree->GetValue(pz).isoValue;
		value5 = aOctree->GetValue(pxz).isoValue;
		value6 = aOctree->GetValue(pyz).isoValue;
		value7 = aOctree->GetValue(pxyz).isoValue;
		material = pData.material;
	}
	else
	{
		value0 = chunksData[(int)p].isoValue;
		value1 = chunksData[(int)px].isoValue;
		value2 = chunksData[(int)py].isoValue;
		value3 = chunksData[(int)pxy].isoValue;
		value4 = chunksData[(int)pz].isoValue;
		value5 = chunksData[(int)pxz].isoValue;
		value6 = chunksData[(int)pyz].isoValue;
		value7 = chunksData[(int)pxyz].isoValue;
		material = chunksData[(int)p].material;
	}

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
	XMFLOAT2 uv = GetUV(material);
	int bits=edgeTable[cubeindex];
	if (bits == 0)
		return;
	float mu = 0.5f;
	if ((bits & 1) != 0)
	{
		mu = (isoLevel - value0) / (value1 - value0);
		vert[0] = lerpSelf(p3, px3, mu);


	}
	if ((bits & 2) != 0)
	{
		mu = (isoLevel - value1) / (value3 - value1);
		vert[1] = lerpSelf(px3, pxy3, mu);
	}
	if ((bits & 4) != 0)
	{
		mu = (isoLevel - value2) / (value3 - value2);
		vert[2] = lerpSelf(py3, pxy3, mu);

	}
	if ((bits & 8) != 0)
	{
		mu = (isoLevel - value0) / (value2 - value0);
		vert[3] = lerpSelf(p3, py3, mu);

	}
	// top of the cube
	if ((bits & 16) != 0)
	{
		mu = (isoLevel - value4) / (value5 - value4);
		vert[4] = lerpSelf(pz3, pxz3, mu);

	}
	if ((bits & 32) != 0)
	{
		mu = (isoLevel - value5) / (value7 - value5);
		vert[5] = lerpSelf(pxz3, pxyz3, mu);

	}
	if ((bits & 64) != 0)
	{
		mu = (isoLevel - value6) / (value7 - value6);
		vert[6] = lerpSelf(pyz3, pxyz3, mu);
	}
	if ((bits & 128) != 0)
	{
		mu = (isoLevel - value4) / (value6 - value4);
		vert[7] = lerpSelf(pz3, pyz3, mu);

	}
	// vertical lines of the cube
	if ((bits & 256) != 0)
	{
		mu = (isoLevel - value0) / (value4 - value0);
		vert[8] = lerpSelf(p3, pz3, mu);

	}
	if ((bits & 512) != 0)
	{
		mu = (isoLevel - value1) / (value5 - value1);
		vert[9] = lerpSelf(px3, pxz3, mu);

	}
	if ((bits & 1024) != 0)
	{
		mu = (isoLevel - value3) / (value7 - value3);
		vert[10] = lerpSelf(pxy3, pxyz3, mu);
	}

	//print (bits & 2048);

	if ((bits & 2048) != 0)
	{
		mu = (isoLevel - value2) / (value6 - value2);
		vert[11] = lerpSelf(py3, pyz3, mu);
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
			vert2.uv = XMFLOAT2(0, 1*tUnit);
			vert3.uv = XMFLOAT2(1 * tUnit, 1 * tUnit);
		}
		else
		{
			vert1.uv = XMFLOAT2(1 * tUnit, 0);
			vert2.uv = XMFLOAT2(0,0);
			vert3.uv = XMFLOAT2(1 * tUnit, 1 * tUnit);
		}
		int vertexIndex = vertices.size();
		vertices.push_back(vert1);
		indices.push_back(vertexIndex);
		vertices.push_back(vert2);
		indices.push_back(vertexIndex + 1);
		vertices.push_back(vert3);
		indices.push_back(vertexIndex + 2);
		i += 3;
	}
}
void VoxelComponent::AddMarchingCase(int x, int y, int z, int _index)
{

}
void VoxelComponent::SubMarchingCase(int x, int y, int z, int _index)
{

}
void VoxelComponent::SetMarchingCubeChunkData(int x, int y, int z,bool isCreate,int length)
{
	
}
void VoxelComponent::SetupMarchingCubeVertexBufferGS()
{
	
}

//마칭큐브 폴리곤 생성

void VoxelComponent::GenerateMarchingCubeFaces(bool isNew)
{
	ULONG time = GetTickCount();
	for (int x = 0; x < width-1; x++)
	{
		for (int y = 0; y < height-1; y++)
		{
			for (int z = 0; z < depth-1; z++)
			{
				CreateFaceMarchingCube(x, y, z, unit);
			}
		}
	}
	printf("Marching Cube Create Buffer ( CPU ): %d ms\n", GetTickCount() - time);
	
}
void VoxelComponent::GenerateMarchingCubeFaces_GS(bool isNew)
{
	ULONG time = GetTickCount();
	if (isNew)
	{
		ULONG time2 = GetTickCount();
		for (int i = 0; i < width-1; i++)
			for (int j = 0; j < height-1; j++)
				for (int k = 0; k < depth-1; k++)
				{
					VertexBuffer vert;
					vert.position = XMFLOAT3(i, j, k);
					vertices.push_back(vert);
				}
		printf("Marching Cube Build Data ( Geometry, useOctree=%d ): %d ms\n", useOctree, GetTickCount() - time2);
	}
	Material* material = renderer->GetMaterial();
	ULONG time3 = GetTickCount();
	//SetupMarchingCubeVertexBufferGS();
	printf("Marching Cube Build Vertex Buffer ( Geometry ): %d ms\n", GetTickCount() - time3);
	if (isNew)
	{
		material->GetParams()->SetInt("vertCount", vertices.size());
	}
	material->GetParams()->SetFloat("isoLevel", isoLevel);
	material->GetParams()->SetInt("length", width);
	ID3D11Device* device = SystemClass::GetInstance()->GetDevice();
	int nums = width * height*depth;
	if(useOctree)
		material->GetParams()->SetSRV("chunksData", new StructuredBuffer(device, sizeof(VoxelData), nums, aOctree->GetNodes(NULL)));
	else material->GetParams()->SetSRV("chunksData", new StructuredBuffer(device,sizeof(VoxelData),nums,chunksData));
	printf("Marching Cube Create Buffer ( Geometry ): %d ms\n", GetTickCount() - time);
}
void VoxelComponent::GenerateMarchingCubeFaces_GPGPU(bool isNew)
{
	ULONG time = GetTickCount();
	if (useOctree)
	{

	}

	for (int x = 0; x < width - 1; x++)
	{
		for (int y = 0; y < height - 1; y++)
		{
			for (int z = 0; z < depth - 1; z++)
			{
				CreateFaceMarchingCube(x, y, z, unit);
			}
		}
	}
	printf("Marching Cube Create Buffer ( GPU ): %d ms\n", GetTickCount() - time);
}
void VoxelComponent::GenerateMarchingCubeFaces_Octree(bool isNew)
{
	ULONG time = GetTickCount();
	for (int x=0;x<width-1;x++)
	{
		for (int y = 0; y < height-1; y++)
		{
			for (int z = 0; z < depth-1; z++)
			{
				CreateFaceMarchingCube(x, y, z, unit);
			}
		}
	}
	printf("Marching Cube Create Buffer ( CPU ): %d ms\n", GetTickCount() - time);
}


void VoxelComponent::GenerateVoxelFaces()
{
	int faceCount = 0;
	short myBlock;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				myBlock = GetChunk(x, y, z).material;
				if (myBlock == 0)
					continue;
				if (GetChunk(x - 1, y, z).material == 0)//LEFT
				{
					CreateFaceLeft(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x + 1, y, z).material == 0)//RIGHT
				{
					CreateFaceRight(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x, y + 1, z).material == 0)//UP
				{
					CreateFaceUp(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x, y - 1, z).material == 0)//DOWN
				{
					CreateFaceDown(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x, y, z + 1).material == 0)//FORWARD
				{
					CreateFaceForward(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
				if (GetChunk(x, y, z - 1).material == 0)//BACKWARD
				{
					CreateFaceBackward(x*unit, y*unit, z*unit,unit, myBlock, faceCount);
				}
			}
		}
	}
}

void VoxelComponent::GenerateOctreeFaces()
{
	int faceCount = 0;
	short myBlock=0;
	VoxelData* nodes;
	int size = 0;
	aOctree->GetNodes(&size, nodes);

	int size1=width;
	int size2=width*height;

	for (int x = 0; x<width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				myBlock = GetChunk(x,y,z).material;
				if (myBlock == 0)
					continue;
				if (GetChunk(x - 1, y, z).material == 0)//LEFT
				{
					CreateFaceLeft(x*unit, y*unit, z*unit, unit, myBlock, faceCount);
				}
				if (GetChunk(x + 1, y, z).material == 0)//RIGHT
				{
					CreateFaceRight(x*unit, y*unit, z*unit, unit, myBlock, faceCount);
				}
				if (GetChunk(x, y + 1, z).material == 0)//UP
				{
					CreateFaceUp(x*unit, y*unit, z*unit, unit, myBlock, faceCount);
				}
				if (GetChunk(x, y - 1, z).material == 0)//DOWN
				{
					CreateFaceDown(x*unit, y*unit, z*unit, unit, myBlock, faceCount);
				}
				if (GetChunk(x, y, z + 1).material == 0)//FORWARD
				{
					CreateFaceForward(x*unit, y*unit, z*unit, unit, myBlock, faceCount);
				}
				if (GetChunk(x, y, z - 1).material == 0)//BACKWARD
				{
					CreateFaceBackward(x*unit, y*unit, z*unit, unit, myBlock, faceCount);
				}
			}
		}
	}
}


void VoxelComponent::CalcNormal(VertexBuffer& v1, VertexBuffer& v2, VertexBuffer& v3)
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


XMFLOAT2 VoxelComponent::GetUV(BYTE type)
{
	if (type == -1)
		return XMFLOAT2(0, 0);
	return XMFLOAT2(tUnit*(type - 1), (int)(type / tAmount) * tUnit);
}

VoxelData VoxelComponent::GetChunk(int x, int y, int z)
{
	VoxelData v;
	v.material = 1;
	if (x >= width || y >= height || z >= depth)
		return v;
	if (x < 0 || y < 0 || z < 0)
		return v;
	if (useOctree&&!chunksData)
	{
		return aOctree->GetValue(XMFLOAT3(x, y, z));
	}
	return chunksData[x + y * width + z * width*height];
}



void VoxelComponent::SetChunk(int x, int y, int z, VoxelData value)
{
	if (x >= width || y >= height || z >= depth)
		return;
	if (x < 0 || y < 0 || z < 0)
		return;
	if (useOctree&&!chunksData)
		aOctree->SetValue(XMFLOAT3(x, y, z), value);
	else chunksData[x + y * width + z * width*height]=value;
}
XMFLOAT3 VoxelComponent::CovertToChunkPos(XMFLOAT3 targetPos, bool returnNan)
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

void VoxelComponent::UpdateMesh(bool isNew)
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

void VoxelComponent::UpdateVoxelMesh()
{
	if (!useOctree)
		GenerateVoxelFaces();
	else
		GenerateOctreeFaces();

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

void VoxelComponent::UpdateMarchingCubeMesh(bool isNew)
{
	if (useMarchingCube)
	{
		if (useGPGPU)
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
	if (!useGeometry || isNew)
	{
		if (mesh)
		{
			mesh->ShutdownBuffers();
			delete mesh;
			mesh = NULL;
		}
		Mesh* newMesh = new Mesh();
		if (vertices.size())
			newMesh->SetVertices(&vertices[0], vertices.size());
		if (indices.size() && !useGeometry)
			newMesh->SetIndices(&indices[0], indices.size());
		if (!useGeometry)
			newMesh->RecalculateNormals();
		newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
		vertices.clear();
		indices.clear();
		mesh = newMesh;
		renderer->SetMesh(newMesh);
	}
}

void VoxelComponent::LoadHeightMapFromRaw(int _width, int _height,int _depth,const char* filename)
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
				VoxelData v;
				v.material = 1;
				SetChunk(x, y, z,v);
			}
		}
	}
	for (int i = 0; i < width; i++)
		delete[] data[i];
	delete[] data;
	printf("%d BYTE chunk data 생성 완료\n", width*depth*height);
}

void VoxelComponent::LoadCube(int _width, int _height, int _depth)
{
	NewChunks(_width, _height, _depth);
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			for (int y = 0; y < height; y++)
			{
				VoxelData v;
				v.material = 1;
				SetChunk(x, y, z, v);
			}
		}
	}
}

void VoxelComponent::LoadPerlin(int _width,int _height, int _depth, int _maxHeight, float refinement)
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
	if (useOctree)
		NewOctree(_width);
	else NewChunks(_width, _height, _depth);
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			float noise = perlin.noise((float)x*refinement, (float)z*refinement, 0);
			noise *= (float)_maxHeight;
			for (int y = 0; y < noise; y++)
			{
				VoxelData vox;
				vox.isoValue = noise-y;
				vox.material = 1;
				SetChunk(x,y,z,vox);
			}
		}
	}
	printf("Load Perlin time : %dms\n", GetTickCount64() - tick);
}

void VoxelComponent::BuildOctree(int _octreelength, XMFLOAT3 basePosition)
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
				if (GetChunk(x, y, z).material)
				{
					XMFLOAT3 pos = XMFLOAT3(x*unit, y*unit, z*unit);// XMFLOAT3(x*unit - oL, y*unit - oL, z*unit - oL);
					//lodLevel = GetLODLevel(basePosition, pos);
					//if (lodLevel < 0)
					//	lodLevel = octree->depth;
					aOctree->SetValue(pos, GetChunk(x,y,z));
				}
			}
		}
	}
	ReleaseChunks();
	printf("Build Octree : %dms\n", GetTickCount64() - tick);
}

void VoxelComponent::NewOctree(int _size)
{
	width = _size;
	height = _size;
	depth = _size;
	if (aOctree)
	{
		delete aOctree;
	}
	aOctree = new ArrayedOctree<VoxelData>(_size);
}

void VoxelComponent::ReadRawEX(unsigned char** &_srcBuf, const char* filename, int _width, int _height)
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
void VoxelComponent::ReadRawEX16(unsigned short** &data, const char* filename, int _width, int _height,int& topY,int &bottomY)
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

int VoxelComponent::GetLODLevel(XMFLOAT3 basePos, XMFLOAT3 targetPos)
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

void VoxelComponent::SetLODLevel(int level, float distance)
{
	if (level < 0 || level >= 8)
		return;
	LODDistance[level] = distance;
}

int VoxelComponent::ReadTXT(const char * filename)
{
	FILE* pInput = NULL;
	int size =1;
	char str[100];
	fopen_s(&pInput, filename, "rb");
	fread(str, sizeof(str)-1, 1, pInput);
	fclose(pInput);
	return atoi(str);
}
