#pragma once
#include"stdafx.h"
#include"VoxelComponent.h"
#include"MeshClass.h"
#include"MeshRenderer.h"
#include"SystemClass.h"
#include"TextureClass.h"
#include"CameraComponent.h"
#include"Octree.h"
#include"ArrayedOctree.h"
#include<vector>
#include<map>
#include<list>
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
#include"VoxelTerrainComponent.h"
#include<process.h>
#include<mutex>
using namespace concurrency;
using namespace concurrency::graphics;
using namespace concurrency::direct3d;

VoxelComponent::VoxelComponent()
{
}

VoxelComponent ::~VoxelComponent()
{
	ReleaseOctree();
}
void VoxelComponent::SetMeshToRenderer(Mesh* newMesh, XMFLOAT3 pos, int depth)
{

	OctreeNode<MeshRenderer*>* rendererNode;
	rendererNode = gOctreeMeshRenderer->root->Subdivide(pos, depth, newMesh!=NULL);

	if (newMesh)
	{
		if (rendererNode->GetValue() == NULL)
		{
			GameObject* gobj = new GameObject("voxel partition");
			MeshRenderer* newRenderer = new MeshRenderer;
			newRenderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_triplanar"));
			gobj->AddComponent(newRenderer);
			rendererNode->SetValue(newRenderer);
		}
		else if (rendererNode->GetValue()->GetMesh())
		{
			rendererNode->GetValue()->GetMesh()->ShutdownBuffers();
			delete rendererNode->GetValue()->GetMesh();
		}
		rendererNode->GetValue()->SetMesh(newMesh);
	}
	else
	{
		if (rendererNode->GetValue())
		{
			if (rendererNode->GetValue()->GetMesh())
			{
				rendererNode->GetValue()->GetMesh()->ShutdownBuffers();
				delete rendererNode->GetValue()->GetMesh();
				rendererNode->GetValue()->SetMesh(NULL);
			}
			GameObject::Destroy(rendererNode->GetValue()->gameObject);
			rendererNode->SetValue((NULL));
		}
	}
}
void VoxelComponent::Update()
{
	ProcessBuildResultQueue();
	ProcessLOD();
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
		//threadPool.AddTask([]()->void {for (int i = 0; i < 1000;i++)printf("test %d\n",i); });
		XMFLOAT3 pos = camera->transform->GetWorldPosition() + camera->transform->forward() * brushRadius;
		XMFLOAT3 cpos = CovertToChunkPos(pos, false);
		int cX = (int)cpos.x;
		int cY = (int)cpos.y;
		int cZ = (int)cpos.z;
		float radius = brushRadius;
		bool isUpdated = false;
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
						VoxelData vox = GetChunk(x, y, z);
						vox.isoValue -= strength * amount;
						if (!useMarchingCube)
						{
							vox.material = 0;
						}
						if (SetChunk((int)x, (int)y, (int)z, vox, true))
						{
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x, y, z), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x - 1, y, z), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x, y - 1, z), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x, y, z - 1), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x - 1, y - 1, z), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x - 1, y, z - 1), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x, y - 1, z - 1), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x - 1, y - 1, z - 1), partitionSize), true);
						}
						//isUpdated = true;
					}
				}
			}
		}
	}
	else if (Input()->GetKey(VK_LBUTTON))
	{
		XMFLOAT3 pos = camera->transform->GetWorldPosition() + camera->transform->forward() *brushRadius;
		XMFLOAT3 cpos = CovertToChunkPos(pos, false);
		int cX = (int)cpos.x;
		int cY = (int)cpos.y;
		int cZ = (int)cpos.z;
		float radius = brushRadius;
		bool isUpdated = false;
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
						VoxelData vox = GetChunk(x, y, z);
						vox.isoValue += strength * amount;
						if (!useMarchingCube)
						{
							vox.material = 1;
						}
						if (SetChunk((int)x, (int)y, (int)z, vox, true))
						{
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x, y, z), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x-1, y, z), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x, y-1, z), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x, y, z-1), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x-1, y-1, z), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x-1, y, z-1), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x, y-1, z-1), partitionSize), true);
							ReserveUpdate(gOctree->GetPartialNode(XMFLOAT3(x-1, y-1,z-1), partitionSize), true);
						}
						//isUpdated = true;
					}
				}
			}
		}
	}
	ProcessUpdateQueue();
	threadPool_Main.ThreadPoolUpdate();
	threadPool_Deform.ThreadPoolUpdate();
}

void VoxelComponent::OnStart()
{
	//transform()->SetPosition(XMFLOAT3(0, 0, 0));
	Initialize();
}
template<> OctreeNode<MeshRenderer*>::~OctreeNode()
{
	if (value)
		GameObject::Destroy(value->gameObject);
	value = 0;
	RemoveChilds();
}
void VoxelComponent::Initialize()
{
	camera = Hierarchy()->FindGameObjectWithName("mainCamera");

	threadPool_Main.Initialize(4,false);
	threadPool_Deform.Initialize(2, false);

	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;

	useMarchingCube = true;
	useAsyncBuild = true;
	//useFrustum = true;

	partitionSize = 8;

	SetLODLevel(0, 150);
	SetLODLevel(1, 200);
	SetLODLevel(2, 300);
	SetLODLevel(3, 500);
	lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	


	//LoadCube(32, 32, 32);
	LoadPerlin(128, 128, 128, 64, 0.3);
	//int h = ReadTXT("/data/height.txt");




	ULONG tick = GetTickCount64();
	//LoadHeightMapFromRaw(1025, 512, 1025, "data/heightmap.r16");// , 0, 0, 255, 255);
	if (!useAsyncBuild)
	{
		currentOctreeDepth = 0;
		UpdateMesh();
	}
	else UpdateMeshAsync(0);
	printf("Init time : %dms\n", GetTickCount64() - tick);

}




void VoxelComponent::ReleaseOctree()
{
	if (gOctree)
		delete gOctree;
	gOctree = 0;
	if (gOctreeMeshRenderer)
		delete gOctreeMeshRenderer;
	gOctreeMeshRenderer = 0;
}
void VoxelComponent::CreateCubeFace_Up(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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


	XMFLOAT3 normal(0, 1, 0);
	v1.normal = normal;
	v2.normal = normal;
	v3.normal = normal;
	v4.normal = normal;

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void VoxelComponent::CreateCubeFace_Down(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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

	XMFLOAT3 normal(0, -1, 0);
	v1.normal = normal;
	v2.normal = normal;
	v3.normal = normal;
	v4.normal = normal;

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void VoxelComponent::CreateCubeFace_Right(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
{

	VertexBuffer v1, v2, v3, v4;
	float offset = _unit * 0.5f;
	v1.position = XMFLOAT3(x + offset, y - offset, z + offset);
	v2.position = XMFLOAT3(x + offset, y + offset, z + offset);
	v3.position = XMFLOAT3(x + offset, y + offset, z - offset);
	v4.position = XMFLOAT3(x + offset, y - offset, z - offset);

	XMFLOAT3 normal(1, 0, 0);
	v1.normal = normal;
	v2.normal = normal;
	v3.normal = normal;
	v4.normal = normal;

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
void VoxelComponent::CreateCubeFace_Left(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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

	XMFLOAT3 normal(-1, 0, 0);
	v1.normal = normal;
	v2.normal = normal;
	v3.normal = normal;
	v4.normal = normal;

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);


	faceCount++;
}
void VoxelComponent::CreateCubeFace_Forward(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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

	XMFLOAT3 normal(0, 0, 1);
	v1.normal = normal;
	v2.normal = normal;
	v3.normal = normal;
	v4.normal = normal;

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void VoxelComponent::CreateCubeFace_Backward(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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

	XMFLOAT3 normal(0, 0, -1);
	v1.normal = normal;
	v2.normal = normal;
	v3.normal = normal;
	v4.normal = normal;

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	faceCount++;
}
void VoxelComponent::CreateMarchingCubeFace(float x, float y, float z, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
	VoxelData pData = gOctree->GetNodeAtPosition(p3, 0)->GetValue();
	value0 = pData.isoValue;
	value1 = gOctree->GetNodeAtPosition(px3, 0)->GetValue().isoValue;
	value2 = gOctree->GetNodeAtPosition(py3, 0)->GetValue().isoValue;
	value3 = gOctree->GetNodeAtPosition(pxy3, 0)->GetValue().isoValue;
	value4 = gOctree->GetNodeAtPosition(pz3, 0)->GetValue().isoValue;
	value5 = gOctree->GetNodeAtPosition(pxz3, 0)->GetValue().isoValue;
	value6 = gOctree->GetNodeAtPosition(pyz3, 0)->GetValue().isoValue;
	value7 = gOctree->GetNodeAtPosition(pxyz3, 0)->GetValue().isoValue;
	material = pData.material;


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
		CalcNormal(vert1, vert2, vert3);
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

VoxelComponent::MESH_RESULT VoxelComponent::GenerateMarchingCubeFaces(XMFLOAT3 pos)
{
	ULONG time = GetTickCount();
	std::vector<OctreeNode<VoxelData>*> leafs;

	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	OctreeNode<VoxelData>* node = gOctree->GetPartialNode(pos, partitionSize);
	node->GetLeafs(leafs);
	for (auto i : leafs)
	{
		XMFLOAT3 cPos = i->GetCenerPosition();
		CreateMarchingCubeFace(cPos.x, cPos.y, cPos.z, unit, vertices, indices);
	}
	printf("Marching Cube Create Buffer ( CPU ): %d ms\n", GetTickCount() - time);
	return CreatePartialMesh(pos, node, vertices, indices);
}



VoxelComponent::MESH_RESULT VoxelComponent::GenerateCubeFaces(XMFLOAT3 pos)
{
	int faceCount = 0;
	short myBlock=0;
	int size = 0;
	std::vector<OctreeNode<VoxelData>*> leafs;

	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	OctreeNode<VoxelData>* node = gOctree->GetPartialNode(pos,partitionSize);
	node->GetLeafs(leafs);
	for (auto i : leafs)
	{
		myBlock = i->GetValue().material;
		if (myBlock == 0)
			continue;
		int x = i->GetCenerPosition().x;
		int y = i->GetCenerPosition().y;
		int z = i->GetCenerPosition().z;
		if (GetChunk(x - 1, y, z,node).material == 0)//LEFT
		{
			CreateCubeFace_Left(x*unit, y*unit, z*unit, unit, myBlock, faceCount,vertices,indices);
		}
		if (GetChunk(x + 1, y, z, node).material == 0)//RIGHT
		{
			CreateCubeFace_Right(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
		if (GetChunk(x, y + 1, z, node).material == 0)//UP
		{
			CreateCubeFace_Up(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
		if (GetChunk(x, y - 1, z, node).material == 0)//DOWN
		{
			CreateCubeFace_Down(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
		if (GetChunk(x, y, z + 1, node).material == 0)//FORWARD
		{
			CreateCubeFace_Forward(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
		if (GetChunk(x, y, z - 1, node).material == 0)//BACKWARD
		{
			CreateCubeFace_Backward(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
	}

	return CreatePartialMesh(pos, node, vertices, indices);
}


void VoxelComponent::CalcNormal(VertexBuffer& v1, VertexBuffer& v2, VertexBuffer& v3)
{
	XMFLOAT3 f1 = v1.position - v2.position;
	//f1 = Normalize3(f1);
	XMFLOAT3 f2 = v3.position - v2.position;
	//f2 = Normalize3(f2);
	XMVECTOR V1 = XMLoadFloat3(&f1);
	V1 = XMVector3Normalize(V1);
	XMVECTOR V2 = XMLoadFloat3(&f2);
	V2 = XMVector3Normalize(V2);
	XMVECTOR UP = XMVector3Cross(V2, V1);
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

VoxelData VoxelComponent::GetChunk(int x, int y, int z, OctreeNode<VoxelData>* _startNode)
{
	VoxelData v;
	v.material = 0;
	if (x >= width || y >= height || z >= depth)
		return v;
	if (x < 0 || y < 0 || z < 0)
		return v;
	if (_startNode)
	{
		if ((x < _startNode->GetPosition().x || x >= _startNode->GetPosition().x + _startNode->GetCellSize())
			|| (y < _startNode->GetPosition().y || y >= _startNode->GetPosition().y + _startNode->GetCellSize())
			|| (z < _startNode->GetPosition().z || z >= _startNode->GetPosition().z + _startNode->GetCellSize()))
			_startNode = NULL;
	}
	v=gOctree->GetNodeAtPosition(XMFLOAT3(x, y, z), 0, _startNode)->GetValue();
	return v;
}



OctreeNode<VoxelData>* VoxelComponent::SetChunk(int x, int y, int z, VoxelData value,bool isDeforming)
{
	if (x >= width || y >= height || z >= depth)
		return NULL;
	if (x < 0 || y < 0 || z < 0)
		return NULL;
	if (gOctree)
	{
		XMFLOAT3 pos(x, y, z);
		VoxelData vox = value;
		VoxelData lastVox;
		lastVox = vox;
		OctreeNode<VoxelData>* node = gOctree->root->Subdivide(pos, 0, true);
		node->SetValue(vox);
		if (isDeforming)
		{
			gOctree->root->Subdivide(pos + XMFLOAT3(-1, 0, 0), 0, true);
			gOctree->root->Subdivide(pos + XMFLOAT3(-1, -1, 0), 0, true);
			gOctree->root->Subdivide(pos + XMFLOAT3(-1, 0, -1), 0, true);
			gOctree->root->Subdivide(pos + XMFLOAT3(-1, -1, -1), 0, true);
			gOctree->root->Subdivide(pos + XMFLOAT3(0, -1, 0), 0, true);
			gOctree->root->Subdivide(pos + XMFLOAT3(0, -1, -1), 0, true);
			gOctree->root->Subdivide(pos + XMFLOAT3(0, 0, -1), 0, true);
		}
		return node;
	}
	return NULL;
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

void VoxelComponent::UpdateMesh()
{
	ULONG tick = GetTickCount64();
	if (!useMarchingCube)
	{
		UpdateVoxelMesh();
	}
	else
	{
		UpdateMarchingCubeMesh();
	}
	printf("Update Mesh : %dms\n", GetTickCount64() - tick);
}

VoxelComponent::MESH_RESULT VoxelComponent::CreatePartialMesh(XMFLOAT3 pos, OctreeNode<VoxelData>* node,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
{
	MESH_RESULT buffer;
	buffer.newMesh = 0;
	buffer.depth = node->GetDepth();
	buffer.pos = node->GetPosition();
	if (vertices.size())
	{
		Mesh* newMesh = new Mesh();
		newMesh->SetVertices(&vertices[0], vertices.size());
		if (indices.size())
			newMesh->SetIndices(&indices[0], indices.size());
		newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
		if (useAsyncBuild)
		{
			buffer.newMesh = newMesh;
		}
		else
			SetMeshToRenderer(newMesh,node->GetPosition(),node->GetDepth());
	}
	else
	{
		SetMeshToRenderer(NULL, node->GetPosition(), node->GetDepth());
	}
	return buffer;
}


VoxelComponent::MESH_RESULT VoxelComponent::UpdatePartialMesh(XMFLOAT3 pos)
{
	MESH_RESULT buffer;
	ULONG tick = GetTickCount64();
	if (!useMarchingCube)
	{
		buffer=GenerateCubeFaces(pos);
	}
	else
	{
		buffer=GenerateMarchingCubeFaces(pos);
	}
	printf("Update Mesh : %dms\n", GetTickCount64() - tick);
	return buffer;
}

void VoxelComponent::UpdateVoxelMesh()
{
	for(int i=0;i<width;i+=partitionSize)
		for (int j = 0; j<height; j += partitionSize)
			for (int k = 0; k<depth; k += partitionSize)
				GenerateCubeFaces(XMFLOAT3(i,j,k));
}

void VoxelComponent::UpdateMarchingCubeMesh()
{
	if (useMarchingCube)
	{
		for (int i = 0; i<width; i += partitionSize)
			for (int j = 0; j<height; j += partitionSize)
				for (int k = 0; k<depth; k += partitionSize)
					GenerateMarchingCubeFaces(XMFLOAT3(i, j, k));
	}
}


void VoxelComponent::ProcessLOD()
{
	return;
}

bool VoxelComponent::FrustumCheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
	for (int i = 0; i<6; i++)
	{
		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter - radius), (yCenter - radius), (zCenter - radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter + radius), (yCenter - radius), (zCenter - radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter - radius), (yCenter + radius), (zCenter - radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter + radius), (yCenter + radius), (zCenter - radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter - radius), (yCenter - radius), (zCenter + radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter + radius), (yCenter - radius), (zCenter + radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter - radius), (yCenter + radius), (zCenter + radius),
			1.0f))) >= 0.0f)
			continue;

		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet((xCenter + radius), (yCenter + radius), (zCenter + radius),
			1.0f))) >= 0.0f)
			continue;

		return false;
	}

	return true;
}

bool VoxelComponent::FrustumCheckSphere(float xCenter, float yCenter, float zCenter, float radius)
{
	for (int i = 0; i<6; i++)
	{
		// 구의 반경이 뷰 frustum 안에 있는지 확인합니다.
		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet(xCenter, yCenter, zCenter, 1.0f))) < -radius)
			return false;
	}

	return true;
}


void VoxelComponent::ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix)
{
	XMFLOAT4X4 pMatrix;
	XMStoreFloat4x4(&pMatrix, projectionMatrix);

	// 절두체에서 최소 Z 거리를 계산합니다.
	float zMinimum = -pMatrix._43 / pMatrix._33;
	float r = screenDepth / (screenDepth - zMinimum);

	// 업데이트 된 값을 다시 투영 행렬에 설정합니다.
	pMatrix._33 = r;
	pMatrix._43 = -r * zMinimum;
	projectionMatrix = XMLoadFloat4x4(&pMatrix);

	// 뷰 매트릭스와 업데이트 된 프로젝션 매트릭스에서 절두체 매트릭스를 만듭니다.
	XMMATRIX finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

	// 최종 행렬을 XMFLOAT4X4 유형으로 변환합니다.
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, finalMatrix);

	// 절두체의 가까운 평면을 계산합니다.
	float x = (float)(matrix._14 + matrix._13);
	float y = (float)(matrix._24 + matrix._23);
	float z = (float)(matrix._34 + matrix._33);
	float w = (float)(matrix._44 + matrix._43);
	m_planes[0] = XMVectorSet(x, y, z, w);
	m_planes[0] = XMPlaneNormalize(m_planes[0]);

	// 절두체의 먼 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[1] = XMVectorSet(x, y, z, w);
	m_planes[1] = XMPlaneNormalize(m_planes[1]);

	// 절두체의 왼쪽 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[2] = XMVectorSet(x, y, z, w);
	m_planes[2] = XMPlaneNormalize(m_planes[2]);

	// 절두체의 오른쪽 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[3] = XMVectorSet(x, y, z, w);
	m_planes[3] = XMPlaneNormalize(m_planes[3]);

	// 절두체의 윗 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[4] = XMVectorSet(x, y, z, w);
	m_planes[4] = XMPlaneNormalize(m_planes[4]);

	// 절두체의 아래 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[5] = XMVectorSet(x, y, z, w);
	m_planes[5] = XMPlaneNormalize(m_planes[5]);

	return;
}



void VoxelComponent::BuildVertexBufferFrustumCulling(int targetDepth)
{
	//int length = width;
	//std::vector<VertexBuffer> vertices;
	//float _unit = unit;
	//if (useOctree)
	//{
	//	_unit = gOctree(targetDepth);//여기
	//	length = aOctree->GetLength(targetDepth);
	//}
	//CameraComponent* camComp=camera->GetComponent<CameraComponent>();
	//XMMATRIX proj, view;
	//camComp->GetProjectionMatrix(proj);
	//camComp->Render();
	//camComp->GetViewMatrix(view);
	//ConstructFrustum(camComp->m_farPlane, proj, view);
	//ULONG time2 = GetTickCount();
	//vertices.reserve(length*length*length);
	//for (int i = 0; i < length - 1; i++)
	//	for (int j = 0; j < length - 1; j++)
	//		for (int k = 0; k < length - 1; k++)
	//		{
	//			XMFLOAT3 pos = XMFLOAT3(i*_unit, j*_unit, k*_unit);
	//			float halfUnit = _unit * 0.5f;
	//			XMFLOAT3 frustumPos = pos + transform()->GetWorldPosition()+XMFLOAT3(halfUnit, halfUnit, halfUnit);
	//			//if (FrustumCheckSphere(frustumPos.x, frustumPos.y, frustumPos.z, halfUnit))
	//			if (FrustumCheckCube(frustumPos.x , frustumPos.y , frustumPos.z, halfUnit))
	//			{
	//				VertexBuffer vert;
	//				vert.position = XMFLOAT3(pos);
	//				vertices.push_back(vert);
	//			}
	//		}
	//unit = _unit;
	//Material* material = renderer->GetMaterial();
	//material->GetParams()->SetInt("vertCount", vertices.size());
	//material->GetParams()->SetInt("length", length);
	//material->GetParams()->SetFloat2("unitSize", XMFLOAT2(unit, 0));
	//printf("Marching Cube Build Data ( Geometry, useOctree=%d ): %d ms\n", useOctree, GetTickCount() - time2);
}

void VoxelComponent::UpdateMeshAsync(int targetDepth)
{
	std::vector<OctreeNode<VoxelData>*> leafs;
	int partialDepth = 0;
	int currentSize = 1;
	while(currentSize<partitionSize)
	{
		partialDepth++;
		currentSize = pow(2, partialDepth);
	}
	gOctree->root->GetLeafs(leafs, (targetDepth>partialDepth)?targetDepth:partialDepth);
	if (leafs.size())
	{
		for (auto i : leafs)
		{
			ReserveUpdate(i,false,false);
		}
	}
}

void VoxelComponent::LoadHeightMapFromRaw(int _width, int _height,int _depth,const char* filename,int startX, int startZ, int endX, int endZ)
{
	NewOctree((startX!=-1&&endX!=-1)?endX-startX+1:_width);
	unsigned short** data = nullptr;
	int top=0,bottom=0;
	ReadRawEX16(data, filename, _width, _depth,top,bottom);
	printf("%d, %d\n", top,bottom);
	float h = (float)height / 65536;
	int mX = (endX!=-1&&endX >= startX && endX < _width) ? endX+1 : _width;
	int mZ = (endZ!=-1&&endZ >= startZ && endZ < _depth) ? endZ+1 : _depth;
	int cX=0, cZ=0;
	for (int x= (startX >= 0 && startX < _width) ? startX : 0; x < mX; x++)
	{
		cZ = 0;
		for (int z= (startZ >= 0 && startZ < _depth) ? startZ : 0; z < mZ; z++)
		{
			float convertY = ((float)data[x][_depth -1-z] * h);
			for (int y = 0; (y < (int)roundl(convertY)); y++)
			{
				VoxelData v;
				v.material = 1;
				v.isoValue = convertY - y;
				SetChunk(cX, y, cZ,v);
			}
			cZ++;
		}
		cX++;
	}
	for (int i = 0; i < width; i++)
		delete[] data[i];
	delete[] data;
	printf("%d BYTE chunk data 생성 완료\n", width*depth*height);
}

void VoxelComponent::LoadCube(int _width, int _height, int _depth)
{
	NewOctree(_width);
	for (int x = 1; x < width-1; x++)
	{
		for (int z = 1; z < depth-1; z++)
		{
			for (int y = 1; y < height-1; y++)
			{
				VoxelData v;
				v.material = 1;
				v.isoValue = height-y-1;
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
	NewOctree(_width);
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			float noise = perlin.noise((float)x*refinement, (float)z*refinement, 0);
			noise *= (float)_maxHeight;
			for (int y = 0; y < _maxHeight; y++)
			{
				VoxelData vox;
				vox.isoValue = noise-y;
				if(y<=noise)
					vox.material = 1;
				OctreeNode<VoxelData>* node=SetChunk(x,y,z,vox);
			}
		}
	}
	printf("Load Perlin time : %dms\n", GetTickCount64() - tick);
}




void VoxelComponent::NewOctree(int _size)
{
	if (_size % 2 > 0)
		_size--;
	width = _size;
	height = _size;
	depth = _size;
	gOctree = new Octree<VoxelData>(_size);
	gOctreeMeshRenderer = new Octree<MeshRenderer*>(_size);
	//if (aOctree)
	//{
	//	delete aOctree;
	//}
	//aOctree = new ArrayedOctree<VoxelData>(_size);
	//for (int i = 0; i < aOctree->depth+1; i++)
	//{
	//	VoxelData vox;
	//	vox.isoValue = -1 * aOctree->GetUnitSize(i);
	//	for (int j = 0; j < aOctree->GetLength(i); j++)
	//	{
	//		aOctree->SetValue(j, vox, i);
	//	}
	//}
}

void VoxelComponent::SetOctreeDepth(int _targetDepth)
{
	currentOctreeDepth = _targetDepth;
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
	XMVECTOR v1, v2;
	float dis;
	v1 = XMVectorSet(basePos.x, basePos.y, basePos.z, 1);
	v2 = XMVectorSet(targetPos.x, targetPos.y, targetPos.z, 1);

	dis = XMVectorGetX(XMVector3Length(XMVectorSubtract(v2, v1)));
	for (int i = 0; i < 4; i++)
		if (LODDistance[i] >= dis)
		{
			return i;
		}
	return 3;
}

void VoxelComponent::SetLODLevel(int level, float distance)
{
	if (level < 0 || level >= 8)
		return;
	LODDistance[level] = distance;
}

void VoxelComponent::ProcessUpdateQueue()
{
	if (!useAsyncBuild)
	{
		for (auto i : updateQueue_Main)
			UpdatePartialMesh(i->GetPosition());
		updateQueue_Main.clear();
		for (auto i : updateQueue_Deform)
			UpdatePartialMesh(i->GetPosition());
		updateQueue_Deform.clear();
	}
	else
	{
		while (updateQueue_Main.size())
		{
			OctreeNode<VoxelData>* _node = updateQueue_Main.front();
			updateQueue_Main.pop_front();
			std::function<MESH_RESULT()> _task = ([&, this, _node]() 
			{ 
				return this->UpdatePartialMesh(_node->GetPosition()); 
			});
			threadPool_Main.AddTask(_task);
		}
		while (updateQueue_Deform.size())
		{
			OctreeNode<VoxelData>* _node = updateQueue_Deform.front();
			updateQueue_Deform.pop_front();
			meshBuildResult.push_back(UpdatePartialMesh(_node->GetPosition()));
			//std::function<MESH_RESULT()> _task = ([&, this, _node]()
			//{
			//	return this->UpdatePartialMesh(_node->GetPosition());
			//});
			//threadPool_Deform.AddTask(_task);
		}
	}
}

void VoxelComponent::ProcessBuildResultQueue()
{
	if (threadPool_Main.isInit)
	{
		if (threadPool_Main.GetResultQueue()->size())
		{
			for (auto i : *(threadPool_Main.GetResultQueue()))
			{
				SetMeshToRenderer(i.newMesh, i.pos, i.depth);
			}
			threadPool_Main.GetResultQueue()->clear();
		}
	}
	if (threadPool_Deform.isInit)
	{
		if (threadPool_Deform.GetResultQueue()->size())
		{
			for (auto i : *(threadPool_Deform.GetResultQueue()))
			{
				SetMeshToRenderer(i.newMesh, i.pos, i.depth);
			}
			threadPool_Deform.GetResultQueue()->clear();
		}
	}
	if (meshBuildResult.size())
	{
		for (auto i : meshBuildResult)
			SetMeshToRenderer(i.newMesh, i.pos, i.depth);
		meshBuildResult.clear();
	}
}

void VoxelComponent::ReserveUpdate(OctreeNode<VoxelData>* _node, bool isDeforming,bool checkDuplicated)
{
	//mutex_UpdateQueue.lock();
	if (checkDuplicated)
	{
		if (isDeforming)
		{
			for (auto i : updateQueue_Deform)
			{
				if (i == _node)
				{
					return;
				}
			}
		}
		else
		{
			for (auto i : updateQueue_Main)
			{
				if (i == _node)
				{
					return;
				}
			}
		}
	}
	if (!isDeforming)
		updateQueue_Main.push_back(_node);
	else
		updateQueue_Deform.push_back(_node);
	//mutex_UpdateQueue.unlock();
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
