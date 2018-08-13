#pragma once
#include"stdafx.h"
#include"VoxelComp.h"
#include"MeshClass.h"
#include"MeshRenderer.h"
#include"SystemClass.h"
#include"TextureClass.h"
#include"CameraComponent.h"
#include"Octree.h"
#include"ArrayedOctree.h"
#include<vector>
#include<map>
#include<algorithm>
#include<list>
#include<time.h>
#include"PerlinNoise.h"
#include<amp.h>
#include<atomic>
#include<amp_graphics.h>
#include"SystemClass.h"
#include"D3DClass.h"
#include"MaterialClass.h"
#include"StructuredBuffer.h"
#include"ResourcesClass.h"
#include"InputClass.h"
#include"HierachyClass.h"
#include"ComputeShader.h"
#include<process.h>
#include<mutex>
using namespace concurrency;
using namespace concurrency::graphics;
using namespace concurrency::direct3d;

VoxelComp::VoxelComp()
{
}

VoxelComp ::~VoxelComp()
{
	delete[] chunks;
	ReleaseOctree();
}
void VoxelComp::Initialize()
{

	camera = Hierarchy()->FindGameObjectWithName("mainCamera");


	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;

	useMarchingCube = true;
	useAsyncBuild = true;
	//useFrustum = true;

	partitionSize = 32;

	SetLODLevel(0, 200);
	SetLODLevel(1, 400);
	SetLODLevel(2, 600);
	SetLODLevel(3, 800);
	lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	


	//LoadCube(32, 32, 32);
	//LoadPerlin(1024, 256, 1024, 200, 0.06);
	//int h = ReadTXT("/data/height.txt");
	LoadHeightMapFromRaw(1025, 512, 1025, "data/heightmap.r16");// , 0, 0, 255, 255);

	std::function<MESH_RESULT(INPUT_BUFFER)> _task = ([&, this](INPUT_BUFFER buf)
	{
		float halfSize = this->GetPartitionSize()*0.5f;
		XMFLOAT3 targetPos = XMFLOAT3(buf.x + halfSize, buf.y + halfSize, buf.z + halfSize);
		int lodLevel=this->GetLODLevel(this->GetLastBasePosition(), targetPos);
		return this->UpdatePartialMesh(XMFLOAT3(buf.x, buf.y, buf.z), lodLevel);
	});
	threadPool_Main.SetTaskFunc(_task);
	threadPool_Deform.SetTaskFunc(_task);
	threadPool_Main.Initialize(8, false);
	threadPool_Deform.Initialize(2, false);





	ULONG tick = GetTickCount64();
	if (!useAsyncBuild)
	{
		currentOctreeDepth = 0;
		UpdateMesh();
	}
	//else UpdateMeshAsync(0);
	printf("Init time : %dms\n", GetTickCount64() - tick);




}
void VoxelComp::UpdateMeshRenderer(Mesh* newMesh, XMFLOAT3 pos,int lodLevel)
{

	OctreeNode<MeshRenderer*>* rendererNode;
	rendererNode = gOctreeMeshRenderer->Subdivide(pos, 0, newMesh!=NULL);
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
			rendererNode->GetValue()->ReleaseMesh();
		}
		rendererNode->GetValue()->SetMesh(newMesh);
	}
	else
	{
		if (rendererNode)
		{
			if (rendererNode->GetValue())
			{
				if (rendererNode->GetValue()->GetMesh())
				{
					rendererNode->GetValue()->ReleaseMesh();
				}
				GameObject::Destroy(rendererNode->GetValue()->gameObject);
				rendererNode->SetValue((NULL));
			}
		}
	}
}
void VoxelComp::Update()
{
	ProcessResultQueue();
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

	if (Input()->GetKeyDown(DIK_F1))
	{
		SystemClass::GetInstance()->GetD3D()->ChangeFillMode(true);
	}
	else if (Input()->GetKeyDown(DIK_F2))
	{
		SystemClass::GetInstance()->GetD3D()->ChangeFillMode(false);
	}
	if (Input()->GetKeyDown(DIK_R))
	{
		lastBasePosition = camera->transform->GetWorldPosition();
		if (!useAsyncBuild)
		{
			UpdateMesh(0);
		}
		else UpdateMeshAsync(0);
	}

	if (Input()->GetKeyDown(DIK_1))
	{
		if (!useAsyncBuild)
		{
			UpdateMesh(0);
		}
		else UpdateMeshAsync(0);
	}
	else if (Input()->GetKeyDown(DIK_2))
	{

		if (!useAsyncBuild)
		{
			UpdateMesh(1);
		}
		else UpdateMeshAsync(0);
	}
	else if (Input()->GetKeyDown(DIK_3))
	{

		if (!useAsyncBuild)
		{
			UpdateMesh(2);
		}
		else UpdateMeshAsync(0);
	}
	else if (Input()->GetKeyDown(DIK_4))
	{

		if (!useAsyncBuild)
		{
			UpdateMesh(3);
		}
		else UpdateMeshAsync(0);
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
						if (SetVoxel((int)x, (int)y, (int)z, vox))
						{
							ReserveUpdate(XMFLOAT3(x, y, z), true);
							ReserveUpdate(XMFLOAT3(x - 1, y, z), true);
							ReserveUpdate(XMFLOAT3(x, y - 1, z), true);
							ReserveUpdate(XMFLOAT3(x, y, z - 1), true);
							ReserveUpdate(XMFLOAT3(x - 1, y - 1, z), true);
							ReserveUpdate(XMFLOAT3(x - 1, y, z - 1), true);
							ReserveUpdate(XMFLOAT3(x, y - 1, z - 1), true);
							ReserveUpdate(XMFLOAT3(x - 1, y - 1, z - 1), true);
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
						if (SetVoxel((int)x, (int)y, (int)z, vox))
						{
							ReserveUpdate(XMFLOAT3(x, y, z), true);
							ReserveUpdate(XMFLOAT3(x - 1, y, z), true);
							ReserveUpdate(XMFLOAT3(x, y - 1, z), true);
							ReserveUpdate(XMFLOAT3(x, y, z - 1), true);
							ReserveUpdate(XMFLOAT3(x - 1, y - 1, z), true);
							ReserveUpdate(XMFLOAT3(x - 1, y, z - 1), true);
							ReserveUpdate(XMFLOAT3(x, y - 1, z - 1), true);
							ReserveUpdate(XMFLOAT3(x - 1, y - 1, z - 1), true);
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

void VoxelComp::OnStart()
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




void VoxelComp::ReleaseOctree()
{
	if (gOctreeMeshRenderer)
		delete gOctreeMeshRenderer;
	gOctreeMeshRenderer = 0;
}
void VoxelComp::CreateCubeFace_Up(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComp::CreateCubeFace_Down(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComp::CreateCubeFace_Right(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComp::CreateCubeFace_Left(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComp::CreateCubeFace_Forward(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComp::CreateCubeFace_Backward(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComp::CreateMarchingCubeFace(XMFLOAT3 pos, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices,bool isTransitionCell)
{
	float size = width;
	float size2 = width * height;
	XMFLOAT3 p3= pos;
	XMFLOAT3 px3 = p3 + XMFLOAT3(1, 0, 0) * _unit;
	XMFLOAT3 py3 = p3 + XMFLOAT3(0, 1, 0) * _unit;
	XMFLOAT3 pxy3 = p3 + XMFLOAT3(1, 1, 0) * _unit;
	XMFLOAT3 pz3 = p3 + XMFLOAT3(0, 0, 1) * _unit;
	XMFLOAT3 pxz3 = p3 + XMFLOAT3(1, 0, 1) * _unit;
	XMFLOAT3 pyz3 = p3 + XMFLOAT3(0, 1, 1) * _unit;
	XMFLOAT3 pxyz3 = p3 + XMFLOAT3(1, 1, 1) * _unit;
	float value0, value1, value2,  value3,  value4,  value5, value6, value7;


	short material = 0;
	VoxelData pData = GetChunk(p3);
	value0 = pData.isoValue;
	value1 = GetChunk(px3).isoValue;
	value2 = GetChunk(py3).isoValue;
	value3 = GetChunk(pxy3).isoValue;
	value4 = GetChunk(pz3).isoValue;
	value5 = GetChunk(pxz3).isoValue;
	value6 = GetChunk(pyz3).isoValue;
	value7 = GetChunk(pxyz3).isoValue;
	material = pData.material;


	int caseCode = 0;

	if (value0 < isoLevel) caseCode |= 1;
	if (value1 < isoLevel) caseCode |= 2;
	if (value2 < isoLevel) caseCode |= 8;
	if (value3 < isoLevel) caseCode |= 4;
	if (value4 < isoLevel) caseCode |= 16;
	if (value5 < isoLevel) caseCode |= 32;
	if (value6 < isoLevel) caseCode |= 128;
	if (value7 < isoLevel) caseCode |= 64;

	XMFLOAT3 vert[12] = { XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0) };
	XMFLOAT2 uv = GetUV(material);
	int bits=edgeTable[caseCode];
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
	while (triTable[caseCode][i] != -1)
	{
		int index1 = triTable[caseCode][i];
		int index2 = triTable[caseCode][i+1];
		int index3 = triTable[caseCode][i + 2];
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

VoxelComp::MESH_RESULT VoxelComp::GeneratePartitionFaces(XMFLOAT3 pos, int lodLevel)
{
	ULONG time = GetTickCount();
	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	pos = XMFLOAT3((int)(pos.x / partitionSize), (int)(pos.y / partitionSize), (int)(pos.z / partitionSize))* partitionSize;
	float  _cellUnit=pow(2,lodLevel);
	for (int i=0;i<partitionSize;i+= _cellUnit)
	{
		for (int j = 0; j < partitionSize; j += _cellUnit)
		{
			for (int k = 0; k < partitionSize; k += _cellUnit)
			{
				if(useMarchingCube)
					CreateMarchingCubeFace(pos + XMFLOAT3(i, j, k), _cellUnit, vertices, indices);
				else
				{

				}
			}
		}
	}
	printf("Marching Cube Create Buffer ( CPU ): %d ms\n", GetTickCount() - time);
	INPUT_BUFFER buffer;
	buffer.x = pos.x;
	buffer.y = pos.y;
	buffer.z = pos.z;
	return CreateNewMesh(buffer, vertices, indices);
}



VoxelComp::MESH_RESULT VoxelComp::GenerateCubeFaces(XMFLOAT3 pos)
{
	return VoxelComp::MESH_RESULT();
	/*int faceCount = 0;
	short myBlock=0;
	int size = 0;
	std::vector<OctreeNode<VoxelData>*> leafs;

	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	OctreeNode<VoxelData>* node = gOctree->GetNodeBySize(pos,partitionSize);
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

	return CreatePartialMesh(pos, node, vertices, indices);*/
}


void VoxelComp::CalcNormal(VertexBuffer& v1, VertexBuffer& v2, VertexBuffer& v3)
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


XMFLOAT2 VoxelComp::GetUV(BYTE type)
{
	if (type == -1)
		return XMFLOAT2(0, 0);
	return XMFLOAT2(tUnit*(type - 1), (int)(type / tAmount) * tUnit);
}

VoxelComp::VoxelData VoxelComp::GetChunk(int x, int y, int z)
{
	VoxelData v;
	if (x >= width || y >= height || z >= depth)
		return v;
	if (x < 0 || y < 0 || z < 0)
		return v;
	v = chunks[x+y*width+z*width*height];
	return v;
}

VoxelComp::VoxelData VoxelComp::GetChunk(XMFLOAT3 pos)
{
	return GetChunk((int)pos.x, (int)pos.y, (int)pos.z);
}



bool VoxelComp::SetVoxel(int x, int y, int z, VoxelData value)
{
	if (x >= width || y >= height || z >= depth)
		return false;
	if (x < 0 || y < 0 || z < 0)
		return false;
	XMFLOAT3 pos(x, y, z);
	VoxelData vox = value;
	chunks[x + y * width + z * width*height] = value;
	return true;
}
XMFLOAT3 VoxelComp::CovertToChunkPos(XMFLOAT3 targetPos, bool returnNan)
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

void VoxelComp::UpdateMesh(int lodLevel)
{
	ULONG tick = GetTickCount64();
	if (!useMarchingCube)
	{
		UpdateVoxelMesh();
	}
	else
	{
		UpdateMarchingCubeMesh(lodLevel);
	}
	printf("Update Mesh : %dms\n", GetTickCount64() - tick);
}

VoxelComp::MESH_RESULT VoxelComp::CreateNewMesh(INPUT_BUFFER input,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
{
	MESH_RESULT buffer;
	buffer.newMesh = 0;
	buffer.pos = XMFLOAT3(input.x,input.y,input.z);
	if (vertices.size())
	{
		Mesh* newMesh = new Mesh();
		newMesh->SetVertices(&vertices[0], vertices.size());
		if (indices.size())
			newMesh->SetIndices(&indices[0], indices.size());
		newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
		buffer.newMesh = newMesh;
	}
	return buffer;
}


VoxelComp::MESH_RESULT VoxelComp::UpdatePartialMesh(XMFLOAT3 pos, int lodLevel)
{
	MESH_RESULT buffer;
	ULONG tick = GetTickCount64();
	if (!useMarchingCube)
	{
		buffer=GenerateCubeFaces(pos);
	}
	else
	{
		buffer=GeneratePartitionFaces(pos, lodLevel);
	}
	printf("Update Mesh : %dms\n", GetTickCount64() - tick);
	return buffer;
}

void VoxelComp::UpdateVoxelMesh()
{
	for(int i=0;i<width;i+=partitionSize)
		for (int j = 0; j<height; j += partitionSize)
			for (int k = 0; k<depth; k += partitionSize)
				GenerateCubeFaces(XMFLOAT3(i,j,k));
}

void VoxelComp::UpdateMarchingCubeMesh(int lodLevel)
{
	if (useMarchingCube)
	{
		for (int i = 0; i<width; i += partitionSize)
			for (int j = 0; j<height; j += partitionSize)
				for (int k = 0; k < depth; k += partitionSize)
				{
					MESH_RESULT buffer = GeneratePartitionFaces(XMFLOAT3(i, j, k), lodLevel);
					UpdateMeshRenderer(buffer.newMesh, buffer.pos,buffer.lodLevel);
				}
	}
}


void VoxelComp::ProcessLOD()
{
	XMFLOAT3 newPosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	std::list<OctreeNode<VoxelData>> lodLevel0;
	//lastBasePosition = newPosition;
	return;
}

bool VoxelComp::FrustumCheckCube(float xCenter, float yCenter, float zCenter, float radius)
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

bool VoxelComp::FrustumCheckSphere(float xCenter, float yCenter, float zCenter, float radius)
{
	for (int i = 0; i<6; i++)
	{
		// 구의 반경이 뷰 frustum 안에 있는지 확인합니다.
		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet(xCenter, yCenter, zCenter, 1.0f))) < -radius)
			return false;
	}

	return true;
}


void VoxelComp::ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix)
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



void VoxelComp::BuildVertexBufferFrustumCulling(int targetDepth)
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

void VoxelComp::UpdateMeshAsync(int lodLevel)
{
	for (int i = 0; i < width; i+=partitionSize)
	{
		for (int j = 0; j < height; j += partitionSize)
		{
			for (int k = 0; k < depth; k += partitionSize)
			{
				ReserveUpdate(XMFLOAT3(i,j,k), false, false);
			}
		}
	}
}

void VoxelComp::LoadHeightMapFromRaw(int _width, int _height,int _depth,const char* filename,int startX, int startZ, int endX, int endZ)
{
	NewChunks(_width, _height, _depth);
	unsigned short** data = nullptr;
	int top=0,bottom=0;
	ReadRawEX16(data, filename, width, depth,top,bottom);
	printf("%d, %d\n", top,bottom);
	float h = (float)height / 65534.0f;
	//int mX = (endX!=-1&&endX >= startX && endX < _width) ? endX+1 : _width;
	//int mZ = (endZ!=-1&&endZ >= startZ && endZ < _depth) ? endZ+1 : _depth;
	//int cX=0, cZ=0;
	//for (int x= (startX >= 0 && startX < _width) ? startX : 0; x < mX; x++)
	//{
	//	cZ = 0;
	//	for (int z= (startZ >= 0 && startZ < _depth) ? startZ : 0; z < mZ; z++)
	//	{
	//		float convertY = ((float)data[x][_depth -1-z] * h);
	//		for (int y = 0; (y < (int)roundl(convertY)); y++)
	//		{
	//			VoxelData v;
	//			v.material = 1;
	//			v.isoValue = convertY - y;
	//			SetVoxel(cX, y, cZ,v);
	//			if (cX%partitionSize == 0 && y%partitionSize == 0 && cZ%partitionSize == 0)
	//				ReserveUpdate(XMFLOAT3(cX, y, cZ), true, false);
	//		}
	//		cZ++;
	//	}
	//	cX++;
	//}
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			float convertY = (data[x][z] * h);
			for (int y = 0; y < convertY; y++)
			{
				VoxelData v;
				v.material = 1;
				v.isoValue = convertY - y;
				SetVoxel(x, y, z, v);
				if (x%partitionSize == 0 && y%partitionSize == 0 && z%partitionSize == 0)
					ReserveUpdate(XMFLOAT3(x, y, z), false, false);
			}
		}
	}

	for (int i = 0; i < width; i++)
		delete[] data[i];
	delete[] data;
	printf("%d BYTE chunk data 생성 완료\n", width*depth*height);
}

void VoxelComp::LoadCube(int _width, int _height, int _depth)
{
	NewChunks(_width, _height, _depth);
	for (int x = 1; x < width; x++)
	{
		for (int z = 1; z < depth; z++)
		{
			for (int y = 1; y < height; y++)
			{
				VoxelData v;
				v.material = 1;
				v.isoValue = height-y-1;
				SetVoxel(x, y, z, v);
				if(x%partitionSize==0&&y%partitionSize==0&&z%partitionSize==0)
					ReserveUpdate(XMFLOAT3(x, y, z), false,false);
			}
		}
	}
}

void VoxelComp::LoadPerlin(int _width,int _height, int _depth, int _maxHeight, float refinement)
{
	ULONG tick = GetTickCount64();
	refinement = refinement / 10;
	PerlinNoise perlin;
	if (_maxHeight > _height)
		_maxHeight = _height *0.5f;
	NewChunks(_width,_height,_depth);
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			float noise = perlin.noise((float)x*refinement, (float)z*refinement, 0);
			noise *= (float)_maxHeight;
			for (int y = 0; y < _maxHeight; y++)
			{
				VoxelData vox;
				vox.isoValue = noise - y;
				if (y <= noise)
					vox.material = 1;
				SetVoxel(x, y, z, vox);
				if (x%partitionSize == 0 && y%partitionSize == 0 && z%partitionSize == 0)
					ReserveUpdate(XMFLOAT3(x, y, z), false, false);
			}
		}
	}
	printf("Load Perlin time : %dms \n", GetTickCount64() - tick);
}


void VoxelComp::NewChunks(int _w, int _h, int _d)
{
	width = _w;
	height = _h;
	depth = _d;
	chunks = new VoxelData[_w*_h*_d];
	NewOctree(_w);
}
void VoxelComp::NewOctree(int _size)
{
	//if (_size % 2 > 0)
	//	_size--;
	int lastSize = 1;
	while (lastSize < _size)
	{
		lastSize *= 2;
	}
	gOctreeMeshRenderer = new Octree<MeshRenderer*>(lastSize,partitionSize);
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

void VoxelComp::SetOctreeDepth(int _targetDepth)
{
	currentOctreeDepth = _targetDepth;
}


void VoxelComp::ReadRawEX(unsigned char** &_srcBuf, const char* filename, int _width, int _height)
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
void VoxelComp::ReadRawEX16(unsigned short** &data, const char* filename, int _width, int _height,int& topY,int &bottomY)
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

int VoxelComp::GetLODLevel(XMFLOAT3 basePos, XMFLOAT3 targetPos)
{
	XMVECTOR v1, v2;
	float dis;
	XMFLOAT3 n = targetPos - basePos;
	for (int i = 0; i < 4; i++)
	{
		if (abs(n.x) <= LODDistance[i]&& abs(n.y) <= LODDistance[i]&& abs(n.z) <= LODDistance[i])
		{
			return i;
		}
	}
	/*v1 = XMVectorSet(basePos.x, basePos.y, basePos.z, 1);
	v2 = XMVectorSet(targetPos.x, targetPos.y, targetPos.z, 1);

	dis = XMVectorGetX(XMVector3Length(XMVectorSubtract(v2, v1)));
	for (int i = 0; i < 4; i++)
		if (LODDistance[i] >= dis)
		{
			return i;
		}*/
	return 3;
}

void VoxelComp::SetLODLevel(int level, float distance)
{
	if (level < 0 || level >= 8)
		return;
	LODDistance[level] = distance;
}

void VoxelComp::ProcessUpdateQueue()
{
	if (!useAsyncBuild)
	{
		if (updateQueue_Main.size())
		{
			ULONG tick = GetTickCount64();
			for (auto i : updateQueue_Main)
				UpdatePartialMesh(XMFLOAT3(i.x, i.y, i.z));
			updateQueue_Main.clear();
			printf("updateQueue_Main Generated %dms\n", GetTickCount64() - tick);
		}
		if (updateQueue_Deform.size())
		{
			ULONG tick = GetTickCount64();
			for (auto i : updateQueue_Deform)
				UpdatePartialMesh(XMFLOAT3(i.x, i.y, i.z));
			updateQueue_Deform.clear();
			printf("updateQueue_Deform Generated %dms\n", GetTickCount64() - tick);
		}
	}
	else
	{
		while (updateQueue_Main.size())
		{
			INPUT_BUFFER _node = updateQueue_Main.front();
			updateQueue_Main.pop_front();
			threadPool_Main.AddTask(_node);
		}
		while (updateQueue_Deform.size())
		{
			INPUT_BUFFER _node = updateQueue_Deform.front();
			updateQueue_Deform.pop_front();
			threadPool_Deform.AddTask(_node);
		}
	}
}

void VoxelComp::ProcessResultQueue()
{
	if (threadPool_Main.isInit)
	{
		if (threadPool_Main.GetResultQueue()->size())
		{
			ULONG tick = GetTickCount64();
			for (auto i : *(threadPool_Main.GetResultQueue()))
			{
				UpdateMeshRenderer(i.newMesh, i.pos,i.lodLevel);
			}
			threadPool_Main.GetResultQueue()->clear();
			printf("threadPool_Main Updated %dms\n", GetTickCount64() - tick);
		}
	}
	if (threadPool_Deform.isInit)
	{
		if (threadPool_Deform.GetResultQueue()->size())
		{
			ULONG tick = GetTickCount64();
			for (auto i : *(threadPool_Deform.GetResultQueue()))
			{
				UpdateMeshRenderer(i.newMesh, i.pos,i.lodLevel);
			}
			threadPool_Deform.GetResultQueue()->clear();
			printf("threadPool_Deform Updated %dms\n", GetTickCount64() - tick);
		}
	}
}

void VoxelComp::ReserveUpdate(XMFLOAT3 pos, bool isDeforming,bool checkDuplicated)
{
	//mutex_UpdateQueue.lock();
	if (pos.x >= width || pos.y >= height || pos.z >= depth)
		return;
	INPUT_BUFFER input;
	input.x = ((int)(pos.x / partitionSize))*partitionSize;
	input.y = ((int)(pos.y / partitionSize))*partitionSize;
	input.z = ((int)(pos.z / partitionSize))*partitionSize;
	if (checkDuplicated)
	{
		if (isDeforming)
		{
			for (auto i : updateQueue_Deform)
				if (i == input)
					return;
		}
		else
		{
			for (auto i : updateQueue_Main)
				if (i == input)
					return;
		}
	}
	if (!isDeforming)
		updateQueue_Main.push_back(input);
	else
		updateQueue_Deform.push_back(input);
	//mutex_UpdateQueue.unlock();
}

int VoxelComp::ReadTXT(const char * filename)
{
	FILE* pInput = NULL;
	int size =1;
	char str[100];
	fopen_s(&pInput, filename, "rb");
	fread(str, sizeof(str)-1, 1, pInput);
	fclose(pInput);
	return atoi(str);
}
