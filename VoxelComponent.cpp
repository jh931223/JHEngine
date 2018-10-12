#pragma once
#include"stdafx.h"
#include"VoxelComponent.h"

#include"SystemClass.h"
#include"D3DClass.h"
#include"SceneClass.h"
#include"ResourcesClass.h"
#include"InputClass.h"
#include"MaterialClass.h"

#include"MeshClass.h"
#include"MeshRenderer.h"
#include"TextureClass.h"
#include"CameraComponent.h"
#include"LightComponent.h"

#include"Octree.h"
#include"JobSystem.h"

#include<algorithm>
#include<unordered_set>
#include<process.h>
#include<mutex>
#include<io.h>
#include<direct.h>
#include <stdio.h>
#include<string>
#include<time.h>

#include"PerlinNoise.h"

#include<amp.h>
#include<amp_math.h>
#include<amp_graphics.h>

#include"VoxelTable.h"

using namespace concurrency;
using namespace concurrency::graphics;
using namespace concurrency::direct3d;
using namespace concurrency::fast_math;

VoxelComponent::VoxelComponent()
{
}

VoxelComponent ::~VoxelComponent()
{
	ReleaseChunks();
}
void VoxelComponent::Initialize()
{

	camera = SceneClass::FindGameObjectWithNameInAllScene("mainCamera");


	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;

	//useFrustum = true;

	info.partitionSize = 16;
//	SetLODLevel(0, 100000);


	SetLODLevel(0, 32);
	SetLODLevel(1, 64);
	SetLODLevel(2, 128);
	SetLODLevel(3, 256);

	//lastBasePosition = XMFLOAT3(0, 30, 0);
	lastBasePosition = XMFLOAT3(3000,3000,3000);
	isLockedBasePosition = true;
	//customPos = lastBasePosition; 


	//LoadCube(32, 32, 32);
	//LoadPerlin(128, 128, 128,128, 0.1f);
	LoadPerlin(256, 128, 256, 128, 0.2f);
	//LoadMapData("Terrain1");
	//int h = ReadTXT("/data/info.height.txt");
	LoadHeightMapFromRaw(1024, 256, 1024,128, "data/terrain.raw");// , 0, 0, 255, 255);

	std::function<RESULT_BUFFER(COMMAND_BUFFER)> _task = ([&, this](COMMAND_BUFFER buf)
	{
		//float halfSize = this->GetPartitionSize()*0.5f;
		//XMFLOAT3 targetPos = XMFLOAT3(buf.x + halfSize, buf.y + halfSize, buf.z + halfSize);
		return this->UpdatePartialMesh(XMFLOAT3(buf.x, buf.y, buf.z), buf.lodLevel,buf.transitionCellBasis);
	});
	threadPool_Main.SetTaskFunc(_task);
	threadPool_Deform.SetTaskFunc(_task);
	//threadPool_Main.Initialize(8, false);
	//threadPool_Deform.Initialize(8, false);





	ULONG tick = GetTickCount64();
	if (!useAsyncBuild)
	{
		currentOctreeDepth = 0;
		UpdateMesh();
	}
	//else UpdateMeshAsync(0);
	printf("Init time : %dms\n", GetTickCount64() - tick);



}
void VoxelComponent::UpdateMeshRenderer(Mesh* newMesh, XMFLOAT3 pos,int lodLevel)
{

	auto chunk = tempChunks->Subdivide(pos, 0, newMesh!=NULL);
	if (newMesh)
	{
		if (chunk->GetValue().renderer == NULL)
		{
			GameObject* gobj = new GameObject("voxel partition");
			MeshRenderer* newRenderer = new MeshRenderer;
			newRenderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_triplanar"));
			gobj->AddComponent(newRenderer);
			if (chunk->GetDepth() != 0)
				chunk = tempChunks->Insert(pos, ChunkData(), 0);
			chunk->GetValue().renderer=newRenderer;
		}
		else if (chunk->GetValue().renderer->GetMesh())
		{
			chunk->GetValue().renderer->ReleaseMesh();
		}
		chunk->GetValue().renderer->SetMesh(newMesh);
	}
	else
	{
		if (chunk)
		{
			if (chunk->GetValue().renderer)
			{
				if (chunk->GetValue().renderer->GetMesh())
				{
					chunk->GetValue().renderer->ReleaseMesh();
				}
				GameObject::Destroy(chunk->GetValue().renderer->gameObject);
				chunk->GetValue().renderer=NULL;
			}
		}
	}
}
void VoxelComponent::Update()
{
	ProcessLOD();
	ProcessResultQueue();
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
	else if (Input()->GetKey(DIK_F3))
	{
		LightComponent::mainLight()->transform()->SetRotation(CameraComponent::mainCamera()->transform()->GetWorldRotation());
	}
	else if (Input()->GetKeyDown(DIK_F4))
	{
		useAsyncBuild = !useAsyncBuild;
	}
	if (Input()->GetKeyDown(DIK_R))
	{
		isLockedBasePosition = true;
		lastBasePosition = GetPartitionStartPos(camera->transform->GetWorldPosition());
		RefreshLODNodes();
		//if (!useAsyncBuild)
		//{
		//	UpdateMesh(0);
		//}
		//else UpdateMeshAsync(0);
	}
	if (Input()->GetKeyDown(DIK_O))
	{
		SaveMapData("Terrain1");
	}
	if (Input()->GetKeyDown(DIK_L))
	{
		isLockedBasePosition = !isLockedBasePosition;
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
		EditVoxel(pos, brushRadius, -strength);

		//for (int x = cpos.x - radius; x < cpos.x + radius; x += 1)
		//{
		//	for (int y = cpos.y - radius; y < cpos.y + radius; y += 1)
		//	{
		//		for (int z = cpos.z - radius; z < cpos.z + radius; z += 1)
		//		{
		//			float dis = GetDistance(XMFLOAT3(x + 0.5f, y + 0.5f, z + 0.5f), cpos);
		//			if (radius >= dis)
		//			{
		//				float amount = 1 - dis / radius;
		//				VoxelData vox = GetVoxel(x, y, z);
		//				vox.isoValue -= strength * amount;
		//				if (!useMarchingCube)
		//				{
		//					vox.material = 0;
		//				}
		//				if (SetVoxel((int)x, (int)y, (int)z, vox))
		//				{
		//					ReserveUpdate(XMFLOAT3(x, y, z), true);
		//					ReserveUpdate(XMFLOAT3(x - 1, y, z), true);
		//					ReserveUpdate(XMFLOAT3(x, y - 1, z), true);
		//					ReserveUpdate(XMFLOAT3(x, y, z - 1), true);
		//					ReserveUpdate(XMFLOAT3(x - 1, y - 1, z), true);
		//					ReserveUpdate(XMFLOAT3(x - 1, y, z - 1), true);
		//					ReserveUpdate(XMFLOAT3(x, y - 1, z - 1), true);
		//					ReserveUpdate(XMFLOAT3(x - 1, y - 1, z - 1), true);
		//				}
		//				//isUpdated = true;
		//			}
		//		}
		//	}
		//}
	}
	else if (Input()->GetKey(VK_LBUTTON))
	{
		XMFLOAT3 pos = camera->transform->GetWorldPosition() + camera->transform->forward() *brushRadius;
		XMFLOAT3 cpos = CovertToChunkPos(pos, false);
		EditVoxel(pos, brushRadius, strength);
		
	}
	ProcessCommandQueue();
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




void VoxelComponent::ReleaseChunks()
{
	if (tempChunks)
		delete tempChunks;
	tempChunks = 0;
	//if (meshRendererOctree)
	//	delete meshRendererOctree;
	//meshRendererOctree = 0;
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
void VoxelComponent::PolygonizeCell(XMFLOAT3 pos, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, XMFLOAT3 min, XMFLOAT3 max)
{
	XMFLOAT3 newCorners[8]=
	{
		XMFLOAT3(min.x,min.y,min.z),
		XMFLOAT3(max.x,min.y,min.z),
		XMFLOAT3(min.x,min.y,max.z),
		XMFLOAT3(max.x,min.y,max.z),
		XMFLOAT3(min.x,max.y,min.z),
		XMFLOAT3(max.x,max.y,min.z),
		XMFLOAT3(min.x,max.y,max.z),
		XMFLOAT3(max.x,max.y,max.z)
	};
	return PolygonizeCell(pos, _unit, vertices, indices, newCorners);
}
void VoxelComponent::PolygonizeCell(XMFLOAT3 pos, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, XMFLOAT3 newCorners[])
{
	if (pos.x + _unit >= info.width || pos.y + _unit >= info.height || pos.z + _unit >= info.depth)
		return;
	float density[8] = { -1,-1,-1,-1,-1,-1,-1,-1 };
	int caseCode = 0;
	for (int i = 0; i < 8; i++)
	{
		density[i] = GetVoxel(pos + regularCorners[i] * _unit).isoValue;
		newCorners[i] = pos + newCorners[i] * _unit;
		if (density[i] < isoLevel)
			caseCode |= 1 << i;
	}
	unsigned char cellClass = regularCellClass[caseCode];
	if (cellClass == 0)
		return;
	RegularCell regularCell = regularCellData[cellClass];
	int vertCount = regularCell.GetVertexCount();
	int triCount = regularCell.GetTriangleCount();
	std::vector<unsigned short> vertDatas;
	for (int i = 0; i < vertCount; i++)
		vertDatas.push_back(regularVertexData[caseCode][i]);
	int vertBegin = vertices.size();
	//std::vector<std::vector<XMFLOAT3>> vertNormalBuffer;
	for (int i = 0; i < vertCount; i++)
	{
		XMFLOAT3 p0, p1;
		int d = vertDatas[i] & 0xFF;
		int d0 = d >> 4;
		int d1 = d & 0xF;
		int reuse = vertDatas[i] >> 8;
		p0 = newCorners[d0];
		p1 = newCorners[d1];
		//XMFLOAT3 pos = p0+(isoLevel - density[d0])*(p1 - p0) / (density[d1] - density[d0]);
		float mu = (isoLevel - density[d0]) / (density[d1] - density[d0]);
		VertexBuffer newVertex;
		//newVertex.position = pos;
		newVertex.position = (lerpSelf(p0, p1, mu));
		newVertex.normal = XMFLOAT3(0, 0, 0);
		//vertNormalBuffer.push_back(std::vector<XMFLOAT3>());
		vertices.push_back(newVertex);
	}
	for (int i = 0; i < triCount; i++)
	{
		int triBegin = i * 3;
		int index_0 = vertBegin + regularCell.Indices()[triBegin];
		int index_1 = vertBegin + regularCell.Indices()[triBegin + 1];
		int index_2 = vertBegin + regularCell.Indices()[triBegin + 2];
		XMFLOAT3 n = CalcNormal(vertices[index_0].position, vertices[index_1].position, vertices[index_2].position);
		vertices[index_0].normal += n;
		vertices[index_1].normal += n;
		vertices[index_2].normal += n;
		indices.push_back(index_0);
		indices.push_back(index_1);
		indices.push_back(index_2);
	}
}




void VoxelComponent::GetVertexInnerBox(short _basis,XMFLOAT3 offset,int _unit,XMFLOAT3 vertOut[])
{
	int max = info.partitionSize - _unit;
	vertOut[0] = XMFLOAT3(0, 0, 0);
	vertOut[1] = XMFLOAT3(1, 0, 0);
	vertOut[2] = XMFLOAT3(0, 0, 1);
	vertOut[3] = XMFLOAT3(1, 0, 1);
	vertOut[4] = XMFLOAT3(0, 1, 0);
	vertOut[5] = XMFLOAT3(1, 1, 0);
	vertOut[6] = XMFLOAT3(0, 1, 1);
	vertOut[7] = XMFLOAT3(1, 1, 1);

	float lowValue = 0.4f;
	float highValue = 0.6f;

	if (_basis & 1)
	{
		vertOut[0].z = lowValue;//뒤
		vertOut[1].z = lowValue;//뒤
		vertOut[4].z = lowValue;//뒤
		vertOut[5].z = lowValue;//뒤
		if (offset.x == 0)
		{
			vertOut[0].z = 0;
			vertOut[4].z = 0;
			if (offset.y == 0)
				vertOut[1].z = 0;
			else if (offset.y == max)
				vertOut[5].z = 0;
		}
		else if (offset.x == max)
		{
			vertOut[1].z = 0;
			vertOut[5].z = 0;
			if (offset.y == 0)
				vertOut[0].z = 0;
			else if (offset.y == max)
				vertOut[4].z = 0;
		}
		else
		{
			if (offset.y == 0)
			{
				vertOut[1].z = 0;
				vertOut[0].z = 0;
			}
			else if (offset.y == max)
			{
				vertOut[4].z = 0;
				vertOut[5].z = 0;
			}
		}
	}
	else if (_basis & 2)
	{
		vertOut[2].z = highValue;//앞
		vertOut[3].z = highValue;//앞
		vertOut[6].z = highValue;//앞
		vertOut[7].z = highValue;//앞
		if (offset.x == 0)
		{
			vertOut[2].z = 1;
			vertOut[6].z = 1;
			if (offset.y == 0)
				vertOut[3].z = 1;
			else if (offset.y == max)
				vertOut[7].z = 1;
		}
		else if (offset.x == max)
		{
			vertOut[3].z = 1;
			vertOut[7].z = 1;
			if (offset.y == 0)
				vertOut[2].z = 1;
			else if (offset.y == max)
				vertOut[6].z = 1;
		}
		else
		{
			if (offset.y == 0)
			{
				vertOut[2].z = 1;
				vertOut[3].z = 1;
			}
			else if (offset.y == max)
			{
				vertOut[6].z = 1;
				vertOut[7].z = 1;
			}
		}
	}
	if (_basis & 4)
	{
		vertOut[0].x = lowValue;
		vertOut[2].x = lowValue;
		vertOut[4].x = lowValue;
		vertOut[6].x = lowValue;
		if (offset.z == 0)
		{
			vertOut[0].x = 0;
			vertOut[4].x = 0;
			if (offset.y == 0)
				vertOut[2].x = 0;
			else if (offset.y == max)
				vertOut[6].x = 0;
		}
		else if (offset.z == max)
		{
			vertOut[2].x = 0;
			vertOut[6].x = 0;
			if (offset.y == 0)
				vertOut[0].x = 0;
			else if (offset.y == max)
				vertOut[4].x = 0;
		}
		else
		{
			if (offset.y == 0)
			{
				vertOut[2].x = 0;
				vertOut[0].x = 0;
			}
			else if (offset.y == max)
			{
				vertOut[4].x = 0;
				vertOut[6].x = 0;
			}
		}
	}
	else if (_basis & 8)
	{
		vertOut[1].x = highValue;
		vertOut[3].x = highValue;
		vertOut[5].x = highValue;
		vertOut[7].x = highValue;
		if (offset.z == 0)
		{
			vertOut[1].x = 1;
			vertOut[5].x = 1;
			if (offset.y == 0)
				vertOut[3].x = 1;
			else if (offset.y == max)
				vertOut[7].x = 1;
		}
		else if (offset.z == max)
		{
			vertOut[3].x = 1;
			vertOut[7].x = 1;
			if (offset.y == 0)
				vertOut[1].x = 1;
			else if (offset.y == max)
				vertOut[5].x = 1;
		}
		else
		{
			if (offset.y == 0)
			{
				vertOut[1].x = 1;
				vertOut[3].x = 1;
			}
			else if (offset.y == max)
			{
				vertOut[7].x = 1;
				vertOut[5].x = 1;
			}
		}
	}
	if (_basis & 16)
	{
		vertOut[0].y = lowValue;
		vertOut[1].y = lowValue;
		vertOut[2].y = lowValue;
		vertOut[3].y = lowValue;
		if (offset.z == 0)
		{
			vertOut[0].y = 0;
			vertOut[1].y = 0;
			if (offset.x == 0)
				vertOut[2].y = 0;
			else if (offset.x == max)
				vertOut[3].y = 0;
		}
		else if (offset.z == max)
		{
			vertOut[2].y = 0;
			vertOut[3].y = 0;
			if (offset.x == 0)
				vertOut[0].y = 0;
			else if (offset.x == max)
				vertOut[1].y = 0;
		}
		else
		{
			if (offset.x == 0)
			{
				vertOut[2].y = 0;
				vertOut[0].y = 0;
			}
			else if (offset.x == max)
			{
				vertOut[1].y = 0;
				vertOut[3].y = 0;
			}
		}
	}
	else if (_basis & 32)
	{
		vertOut[4].y = highValue;
		vertOut[5].y = highValue;
		vertOut[6].y = highValue;
		vertOut[7].y = highValue;
		if (offset.z == 0)
		{
			vertOut[4].y = 1;
			vertOut[5].y = 1;
			if (offset.x == 0)
				vertOut[6].y = 1;
			else if (offset.x == max)
				vertOut[7].y = 1;
		}
		else if (offset.z == max)
		{
			vertOut[6].y = 1;
			vertOut[7].y = 1;
			if (offset.x == 0)
				vertOut[4].y = 1;
			else if (offset.x == max)
				vertOut[5].y = 1;
		}
		else
		{
			if (offset.x == 0)
			{
				vertOut[4].y = 1;
				vertOut[6].y = 1;
			}
			else if (offset.x == max)
			{
				vertOut[5].y = 1;
				vertOut[7].y = 1;
			}
		}
	}


}
void VoxelComponent::PolygonizeTransitionCell(XMFLOAT3 pos, int _unit, short _basis, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
{
	if (pos.x + _unit >= info.width || pos.y + _unit >= info.height || pos.z + _unit >= info.depth)
		return;
	int caseCode = 0;
	std::vector<XMFLOAT3> newCorners;
	XMFLOAT3 startPos = GetPartitionStartPos(pos);
	XMFLOAT3 offset=pos - startPos;
	XMFLOAT3 vertOut[8];
	GetVertexInnerBox(_basis, offset, _unit, vertOut);
	XMFLOAT3 _9abc[6][4]=
	{
		{ vertOut[0],vertOut[1],vertOut[4],vertOut[5] },
		{ vertOut[3],vertOut[2],vertOut[7],vertOut[6] },
		{ vertOut[2],vertOut[0],vertOut[6],vertOut[4] },
		{ vertOut[1],vertOut[3],vertOut[5],vertOut[7] },
		{ vertOut[2],vertOut[3],vertOut[0],vertOut[1] },
		{ vertOut[4],vertOut[5],vertOut[6],vertOut[7] },
	}; 
	PolygonizeCell(pos, _unit, vertices, indices, vertOut);
	//PolygonizeCell(pos, _unit, vertices, indices, innerBoxMin, innerBoxMax);
	int basis = _basis;
	for(int b=0;b<6;b++)
	{
		if (basis & (1 << b))
		{
			newCorners.clear();
			float density[13] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
			for (int j=0;j<9;j++)
			{
				newCorners.push_back(pos + transitionCornersByBasis[b][j] * _unit);
				density[j] = GetVoxel(newCorners[j]).isoValue;
			}
			for (int j = 0; j < 4; j++)
				newCorners.push_back(pos + _9abc[b][j] * _unit);

			density[9] = density[0];
			density[10] = density[2];
			density[11] = density[6];
			density[12] = density[8];

			caseCode = 0;
			if (density[0] < isoLevel)caseCode |= 1 << 0;
			if (density[1] < isoLevel)caseCode |= 1 << 1;
			if (density[2] < isoLevel)caseCode |= 1 << 2;
			if (density[3] < isoLevel)caseCode |= 1 << 7;
			if (density[4] < isoLevel)caseCode |= 1 << 8;
			if (density[5] < isoLevel)caseCode |= 1 << 3;
			if (density[6] < isoLevel)caseCode |= 1 << 6;
			if (density[7] < isoLevel)caseCode |= 1 << 5;
			if (density[8] < isoLevel)caseCode |= 1 << 4;


			unsigned char cellClass = transitionCellClass[caseCode] & 0x7F;
			if (cellClass == 0)
				continue;
			bool inverse = (transitionCellClass[caseCode] & 128) ? false : true;
			TransitionCell cell = transitionCellData[cellClass];
			int vertCount = cell.GetVertexCount();
			int triCount = cell.GetTriangleCount();
			std::vector<unsigned short> vertDatas;
			for (int i = 0; i < vertCount; i++)
				vertDatas.push_back(transitionVertexData[caseCode][i]);
			int vertBegin = vertices.size();

			for (int i = 0; i < vertCount; i++)
			{
				XMFLOAT3 p0, p1;
				int d = vertDatas[i] & 0xFF;
				unsigned int d0 = d >> 4;
				unsigned int d1 = d & 0xF;
				int reuse = vertDatas[i] >> 8;
				//if (transitionCornerOrder[basis][d0]<transitionCornerOrder[basis][d1])
				//{
				//	swap<unsigned int>(d0, d1);
				//}
				p0 = newCorners[d0];
				p1 = newCorners[d1];
				float mu = (isoLevel - density[d0]) / (density[d1] - density[d0]);
				VertexBuffer newVertex;
				newVertex.position = (lerpSelf(p0, p1, mu));
				newVertex.normal = XMFLOAT3(0, 1, 0);
				vertices.push_back(newVertex);
			}
			int indexBegin = indices.size();
			for (int i = 0; i < triCount; i++)
			{
				int triBegin = i * 3;
				int index_0 = vertBegin + cell.Indices()[triBegin + ((inverse) ? 2 : 0)];
				int index_1 = vertBegin + cell.Indices()[triBegin + 1];
				int index_2 = vertBegin + cell.Indices()[triBegin + ((inverse) ? 0 : 2)];
				XMFLOAT3 n = CalcNormal(vertices[index_0].position, vertices[index_1].position, vertices[index_2].position);
				vertices[index_0].normal += n;
				vertices[index_1].normal += n;
				vertices[index_2].normal += n;
				indices.push_back(index_0);
				indices.push_back(index_1);
				indices.push_back(index_2);
			}
		}
	}


}

RESULT_BUFFER VoxelComponent::GeneratePartitionFaces(XMFLOAT3 pos, int lodLevel, short transitionCellBasis)
{
	//ULONG time = GetTickCount();
	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	pos = GetPartitionStartPos(pos);
	float  _cellUnit=pow(2,lodLevel);
	int max = info.partitionSize - _cellUnit;
	//std::map<std::vector<
	for (int i=0;i<info.partitionSize && (pos.x + i<info.width);i+= _cellUnit)
	{
		for (int j = 0; j < info.partitionSize && (pos.y + j<info.height); j += _cellUnit)
		{
			for (int k = 0; k < info.partitionSize&&(pos.z + k<info.depth); k += _cellUnit)
			{
				if (useMarchingCube)
				{

					short basis = 0;
					if (transitionCellBasis & 8)
					{
						transitionCellBasis |= 8;
					}
					if (transitionCellBasis)
					{
						if ((transitionCellBasis & 1) && k == 0)
							basis |= 1;
						if ((transitionCellBasis & 2) && k == max)
							basis |= 2;
						if ((transitionCellBasis & 4) && i == 0)
							basis |= 4;
						if ((transitionCellBasis & 8) && i == max)
							basis |= 8;
						if ((transitionCellBasis & 16) && j == 0)basis |= 16;
						if ((transitionCellBasis & 32) && j == max)basis |= 32;
					}
					if (lodLevel>0&&basis)
					{
						transitionCellBasis = transitionCellBasis;
						PolygonizeTransitionCell(pos + XMFLOAT3(i, j, k), _cellUnit, basis, vertices, indices);
					}
					else
						PolygonizeCell(pos + XMFLOAT3(i, j, k), _cellUnit,vertices, indices);
				}
			}
		}
	}
	for (int i = 0; i < vertices.size();i++)
	{

	}
	//printf("vertices = %d\n", vertices.size());
	//printf("Marching Cube Create Buffer ( CPU ): %d ms x:%f y:%f z:%f\n", GetTickCount() - time,pos.x,pos.y,pos.z);
	COMMAND_BUFFER buffer;
	buffer.x = pos.x;
	buffer.y = pos.y;
	buffer.z = pos.z;
	return CreateNewMesh(buffer, vertices, indices);
}


RESULT_BUFFER VoxelComponent::GenerateCubeFaces(XMFLOAT3 pos)
{
	return RESULT_BUFFER();
	/*int faceCount = 0;
	short myBlock=0;
	int size = 0;
	std::vector<OctreeNode<VoxelData>*> leafs;

	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	OctreeNode<VoxelData>* node = gOctree->GetNodeBySize(pos,info.partitionSize);
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

XMFLOAT3 VoxelComponent::CalcNormal(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3)
{
	XMFLOAT3 f1 = v1 - v2;
	//f1 = Normalize3(f1);
	XMFLOAT3 f2 = v3 - v2;
	//f2 = Normalize3(f2);
	XMVECTOR V1 = XMLoadFloat3(&f1);
	V1 = XMVector3Normalize(V1);
	XMVECTOR V2 = XMLoadFloat3(&f2);
	V2 = XMVector3Normalize(V2);
	XMVECTOR UP = XMVector3Cross(V2, V1);
	UP = XMVector3Normalize(UP);
	XMFLOAT3 n;
	XMStoreFloat3(&n, UP);
	return n;
}

XMFLOAT2 VoxelComponent::GetUV(BYTE type)
{
	if (type == -1)
		return XMFLOAT2(0, 0);
	return XMFLOAT2(tUnit*(type - 1), (int)(type / tAmount) * tUnit);
}

VoxelComponent::VoxelData VoxelComponent::GetVoxel(int x, int y, int z)
{
	VoxelData v;
	if (x >= info.width || y >= info.height || z >= info.depth)
		return v;
	if (x < 0 || y < 0 || z < 0)
		return v;
	int index = GetCellIndex(x, y, z);
	XMFLOAT3 pos = XMFLOAT3(x, y, z);
	auto node = tempChunks->GetNodeAtPosition(pos, 0);
	if (node->GetDepth() == 0&&node->GetValue().IsHaveChunk())
	{
		//node=tempChunks->Insert(pos, new std::vector<VoxelData>(info.partitionSize*info.partitionSize*info.partitionSize), 0);
		x = x - (int)node->GetPosition().x;
		y = y - (int)node->GetPosition().y;
		z = z - (int)node->GetPosition().z;
		v = node->GetValue().chunk[x+y*info.partitionSize+z*info.partitionSize*info.partitionSize];
	}

	return v;
}

VoxelComponent::VoxelData VoxelComponent::GetVoxel(XMFLOAT3 pos)
{
	return GetVoxel((int)pos.x, (int)pos.y, (int)pos.z);
}

unsigned int VoxelComponent::GetCellIndex(int x, int y, int z)
{
	return x + y * info.width + z * info.width*info.height;
}
bool VoxelComponent::SetVoxel(int x, int y, int z, VoxelData value,bool isInit)
{
	if (x >= info.width || y >= info.height || z >= info.depth)
		return false;
	if (x < 0 || y < 0 || z < 0)
		return false;
	XMFLOAT3 pos(x, y, z);
	VoxelData vox = value;
	int index = GetCellIndex(x, y, z);

	auto node = tempChunks->GetNodeAtPosition(pos, 0);
	if (node->GetDepth() != 0)
		node = tempChunks->Insert(pos,ChunkData(), 0);
	if (!node->GetValue().IsHaveChunk())
		node->GetValue().MakeChunk(GetPartitionSize());
	x = x - (int)node->GetPosition().x;
	y = y - (int)node->GetPosition().y;
	z = z - (int)node->GetPosition().z;
	int idx = x + y*info.partitionSize + z*info.partitionSize*info.partitionSize;
	node->GetValue().chunk[idx]=value;
	node->GetValue().chunk[idx] = node->GetValue().chunk[idx];

	return true;
}

bool VoxelComponent::EditVoxel(XMFLOAT3 pos, float _radius, float _strength)
{
	XMFLOAT3 partitionPos = GetPartitionStartPos(pos);
	float hUnit = unit * 0.5f;
	pos = XMFLOAT3((int)pos.x + hUnit, (int)pos.y + hUnit, (int)pos.z + hUnit);
	std::unordered_set<int> reservedSet;
	for (int i = -_radius-1; i < _radius+1; i++)
	{
		for (int j = -_radius-1; j < _radius+1; j++)
		{
			for (int k = -_radius-1; k < _radius+1; k++)
			{
				float dis = GetDistance(pos, pos + XMFLOAT3(i, j, k));
				/*if (dis <= _radius)
				{*/
					float amount = _radius - dis;//1 -dis / _radius;
					XMFLOAT3 nPos(pos.x + i - hUnit, pos.y + j - hUnit, pos.z + k - hUnit);
					VoxelData data = GetVoxel(nPos);
					float lastValue = data.isoValue;
					data.isoValue = amount*SIGN(_strength);
					//data.isoValue += amount*_strength;
					//data.isoValue = data.isoValue + _strength * amount;
					if (_strength > 0 && lastValue > data.isoValue)
						continue;
					else if (_strength < 0 && lastValue < data.isoValue)
						continue;
					else if (lastValue == data.isoValue)
						continue;
					if (SetVoxel(nPos.x, nPos.y, nPos.z, data, false))
					{

						ReserveUpdate(nPos, Reserve_Deform);
						XMFLOAT3 pPos = GetPartitionStartPos(nPos);
						if(pPos.x == nPos.x)
							ReserveUpdate(nPos + XMFLOAT3(-1, 0, 0), Reserve_Deform);
						if (pPos.y == nPos.y)
							ReserveUpdate(nPos + XMFLOAT3(0, -1, 0), Reserve_Deform);
						if (pPos.x == nPos.x&&pPos.y == nPos.y)
							ReserveUpdate(nPos + XMFLOAT3(-1, -1, 0), Reserve_Deform);
						if (pPos.x == nPos.x&&pPos.y == nPos.y&&pPos.z==nPos.z)
							ReserveUpdate(nPos + XMFLOAT3(-1, -1, -1), Reserve_Deform);
						if (pPos.y == nPos.y&&pPos.z == nPos.z)
							ReserveUpdate(nPos + XMFLOAT3(0, -1, -1), Reserve_Deform);
						if (pPos.x == nPos.x&&pPos.z == nPos.z)
							ReserveUpdate(nPos + XMFLOAT3(-1, 0, -1), Reserve_Deform);
						if (pPos.z == nPos.z)
							ReserveUpdate(nPos + XMFLOAT3(0, 0, -1), Reserve_Deform);
					}
				//}
			}
		}
	}
	return true;
}

XMFLOAT3 VoxelComponent::CovertToChunkPos(XMFLOAT3 targetPos, bool returnNan)
{
	XMFLOAT3 newpos = transform()->GetWorldPosition();
	XMFLOAT3 startPos = newpos;// - XMFLOAT3(info.width*0.5f, info.width*0.5f, info.width*0.5f);
	XMFLOAT3 endPos = newpos + XMFLOAT3(info.width, info.height, info.depth);
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

void VoxelComponent::UpdateMesh(int lodLevel)
{
	//ULONG tick = GetTickCount64();
	if (!useMarchingCube)
	{
		UpdateVoxelMesh();
	}
	else
	{
		UpdateMarchingCubeMesh(lodLevel);
	}
	//printf("Update Mesh : %dms\n", GetTickCount64() - tick);
}

RESULT_BUFFER VoxelComponent::CreateNewMesh(COMMAND_BUFFER input,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
{
	RESULT_BUFFER buffer;
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


RESULT_BUFFER VoxelComponent::UpdatePartialMesh(XMFLOAT3 pos, int lodLevel, short transitionCellBasis)
{
	RESULT_BUFFER buffer;
	//ULONG tick = GetTickCount64();
	if (!useMarchingCube)
	{
		buffer=GenerateCubeFaces(pos);
	}
	else
	{
		buffer=GeneratePartitionFaces(pos, lodLevel,transitionCellBasis);
	}
	//printf("Update Mesh : %dms\n", GetTickCount64() - tick);
	return buffer;
}

void VoxelComponent::UpdateVoxelMesh()
{
	for(int i=0;i<info.width;i+=info.partitionSize)
		for (int j = 0; j<info.height; j += info.partitionSize)
			for (int k = 0; k<info.depth; k += info.partitionSize)
				GenerateCubeFaces(XMFLOAT3(i,j,k));
}

void VoxelComponent::UpdateMarchingCubeMesh(int lodLevel)
{
	ProcessCommandQueue();
	/*if (useMarchingCube)
	{
		for (int i = 0; i<info.width; i += info.partitionSize)
			for (int j = 0; j<info.height; j += info.partitionSize)
				for (int k = 0; k < info.depth; k += info.partitionSize)
				{
					RESULT_BUFFER buffer = GeneratePartitionFaces(XMFLOAT3(i, j, k), lodLevel);
					UpdateMeshRenderer(buffer.newMesh, buffer.pos,buffer.lodLevel);
				}
	}*/
}
XMFLOAT3 VoxelComponent::GetPositionFromIndex(int index)
{
	int x = index % info.width;
	int y = (index / info.width) % info.height;
	int z = index / (info.width* info.height);
	return XMFLOAT3(x, y, z);
}
unsigned int VoxelComponent::GetIndexFromPosition(XMFLOAT3 pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.z < 0)
		return -1;
	if (pos.x >= info.width || pos.y >= info.height || pos.z >= info.depth)
		return -1;
	return (int)pos.x + (int)pos.y*info.width + (int)pos.z*info.width*info.height;
}
void VoxelComponent::RefreshLODNodes()
{
	/*
	XMFLOAT3 basePos = GetPartitionStartPos(lastBasePosition);
	std::unordered_map<int, LODGroupData> newLODGroupsLoaded;
	std::unordered_map<int, LODGroupData> newLODGroups;
	std::unordered_map<int, LODGroupData> oldLODGroups;
	oldLODGroups.swap(lodGropups);
	int lodDistance = LODDistance[maxLODLevel - 1];
	int min = -lodDistance - info.partitionSize;
	int max = lodDistance;

	for (int i = min; i <= max; i += info.partitionSize)
	{
		for (int j = min; j <= max; j += info.partitionSize)
		{
			for (int k = min; k <= max; k += info.partitionSize)
			{
				XMFLOAT3 targetPos = basePos + XMFLOAT3(i, j, k);
				if (targetPos.x < 0 || targetPos.y < 0 || targetPos.z < 0 || targetPos.x >= info.width || targetPos.y >= info.height || targetPos.z >= info.depth)
					continue;

				LODGroupData lodGroupData;
				lodGroupData.level = GetLODLevel(basePos, targetPos);

				lodGroupData.transitionBasis = GetTransitionBasis(lodGroupData.level, basePos, targetPos);
				int index = (int)targetPos.x + (int)targetPos.y*info.width + (int)targetPos.z*info.width*info.height;
				auto iter = oldLODGroups.find(index);
				if (iter == oldLODGroups.end())
				{
					//if ((i == min && j == min && k == min)
					//	|| (i == max && j == min && k == min)
					//	|| (i == min && j == max && k == min)
					//	|| (i == min && j == min && k == max)
					//	|| (i == max && j == max && k == min)
					//	|| (i == max && j == min && k == max)
					//	|| (i == min && j == max && k == max)
					//	|| (i == max && j == max && k == max))
					//	continue;
					newLODGroups[index] = lodGroupData;
				}
				else
				{
					newLODGroups[index] = lodGroupData;
					oldLODGroups.erase(iter);
				}
			}
		}
	}

	for (auto i : newLODGroupsLoaded)
	{
		lodGropups[i.first] = i.second;
	}
	for (auto i : newLODGroups)
	{
		ReserveUpdate(GetPositionFromIndex(i.first), i.second.transitionBasis, i.second.level, false, false);
		lodGropups[i.first] = i.second;
	}

	for (auto i : oldLODGroups)
	{
		if (ReserveUpdate(GetPositionFromIndex(i.first), false, false) == maxLODLevel)
		{
			int x = i.second.level;
			int y = i.second.transitionBasis;
		}
	}
	oldLODGroups.clear();
	*/
	for (auto i : lodGropups)
	{
		ReserveUpdate(GetPositionFromIndex(i.first), Reserve_LOD, true);
	}
	lodGropups.clear();
	std::unordered_map<int, LODGroupData> newLODGroups;
	XMFLOAT3 basePos = GetPartitionStartPos(lastBasePosition);
	int lodDistance = LODDistance[maxLODLevel - 1];
	int min = -lodDistance - info.partitionSize;
	int max = lodDistance;
	for (int i = min; i <= max; i += info.partitionSize)
		for (int j = min; j <= max; j += info.partitionSize)
			for (int k = min; k <= max; k += info.partitionSize)
			{
				XMFLOAT3 targetPos = XMFLOAT3(i, j, k) + basePos;
				int level = GetLODLevel(basePos, targetPos);//ReserveUpdate(targetPos, false, false);
				if (level >= 0)
				{
					unsigned int index = GetIndexFromPosition(targetPos);
					LODGroupData data;
					data.level = level;
					data.transitionBasis = GetTransitionBasis(level, basePos, targetPos);
					auto iter = lodGropups.find(index);
					if (iter != lodGropups.end())
					{
						if (iter->second.level == data.level&&iter->second.transitionBasis == data.transitionBasis)
						{

						}
					}
					newLODGroups[GetIndexFromPosition(targetPos)] = data;
					ReserveUpdate(targetPos, data.transitionBasis, data.level, Reserve_LOD, true);
				}
			}
	newLODGroups.swap(lodGropups);
}
void VoxelComponent::RefreshLODNodes(XMFLOAT3 basePos)
{


	basePos = GetPartitionStartPos(basePos);
	std::unordered_map<int, LODGroupData> newLODGroupsLoaded;
	std::unordered_map<int, LODGroupData> newLODGroups;
	std::unordered_map<int, LODGroupData> oldLODGroups;
	oldLODGroups.swap(lodGropups);
	int lodDistance = LODDistance[maxLODLevel - 1];
	int min = -lodDistance-info.partitionSize;
	int max = lodDistance+info.partitionSize;

	for (int i = min; i < max; i+=info.partitionSize)
	{
		for (int j= min; j < max; j+=info.partitionSize)
		{
			for (int k = min; k < max; k+=info.partitionSize)
			{
				XMFLOAT3 targetPos = basePos + XMFLOAT3(i, j, k);
				if (targetPos.x < 0 || targetPos.y < 0 || targetPos.z < 0 || targetPos.x >= info.width || targetPos.y >= info.height || targetPos.z >= info.depth)
					continue;

				LODGroupData lodGroupData;
				lodGroupData.level = GetLODLevel(basePos, targetPos);

				lodGroupData.transitionBasis = GetTransitionBasis(lodGroupData.level, basePos, targetPos);
				int index = (int)targetPos.x + (int)targetPos.y*info.width + (int)targetPos.z*info.width*info.height;
				auto iter = oldLODGroups.find(index);
				if (iter == oldLODGroups.end())
				{
					newLODGroups[index] = lodGroupData;
				}
				else
				{
					if (lodGroupData.transitionBasis == iter->second.transitionBasis&&lodGroupData.level == iter->second.level)
						newLODGroupsLoaded[index] = lodGroupData;
					else
					{
						oldLODGroups.erase(iter);
						newLODGroups[index] = lodGroupData;
					}
				}
			}
		}
	}

	for (auto i : newLODGroupsLoaded)
	{
		lodGropups[i.first] = i.second;
	}
	for (auto i : newLODGroups)
	{
		ReserveUpdate(GetPositionFromIndex(i.first),i.second.transitionBasis,i.second.level, Reserve_LOD, false);
		lodGropups[i.first] = i.second;
	}
	for (auto i : oldLODGroups)
	{
		ReserveUpdate(GetPositionFromIndex(i.first), Reserve_LOD, false);
	}
	oldLODGroups.clear();

}

void VoxelComponent::ProcessLOD()
{
	if (isLockedBasePosition)
		return;
	XMFLOAT3 newPosition = GetPartitionStartPos(CameraComponent::mainCamera()->transform()->GetWorldPosition());
	if (newPosition!=lastBasePosition)
	{
		lastBasePosition = newPosition;
		RefreshLODNodes(newPosition);
	}
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
	//int length = info.width;
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

void VoxelComponent::UpdateMeshAsync(int lodLevel)
{
	for (int i = 0; i < info.width; i+=info.partitionSize)
	{
		for (int j = 0; j < info.height; j += info.partitionSize)
		{
			for (int k = 0; k < info.depth; k += info.partitionSize)
			{
				ReserveUpdate(XMFLOAT3(i,j,k), Reserve_Load,false);
			}
		}
	}
}

void VoxelComponent::LoadHeightMapFromRaw(int _width, int _height,int _depth, int _maxHeight, const char* filename,int startX, int startZ, int endX, int endZ)
{
	NewChunks(_width, _height, _depth);
	unsigned short** data = nullptr;
	int top=0,bottom=0;
	ReadRawEX16(data, filename, info.width, info.depth,top,bottom);
	printf("%d, %d\n", top,bottom);
	float h = ((float)_maxHeight) / 65534.0f;
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
	//			if (cX%info.partitionSize == 0 && y%info.partitionSize == 0 && cZ%info.partitionSize == 0)
	//				ReserveUpdate(XMFLOAT3(cX, y, cZ), true, false);
	//		}
	//		cZ++;
	//	}
	//	cX++;
	//}
	for (int x = 0; x < info.width; x++)
	{
		for (int z = 0; z < info.depth; z++)
		{
			float convertY = (((float)data[x][z]) * h);
			for (int y = 0; y <= _maxHeight; y++)
			{
				VoxelData v;
				if (y <= convertY)
				{
					v.material = 1;
				}
				v.isoValue = convertY - y;
				SetVoxel(x, y, z, v, true);
				if ((x%info.partitionSize == 0) && (y%info.partitionSize == 0) && (z%info.partitionSize == 0))
					ReserveUpdate(XMFLOAT3(x, y, z), Reserve_Load, false);
			}
		}
	}

	for (int i = 0; i < info.width; i++)
		delete[] data[i];
	delete[] data;
	printf("%d BYTE chunk data 생성 완료\n", info.width*info.depth*info.height);
}
void VoxelComponent::GetFileNamesInDirectory(const char* path, std::vector<std::string>& vstr)
{
	_finddata_t fd;
	long handle = _findfirst(path, &fd);

	if (handle == -1) return;

	do {
		vstr.push_back(fd.name);
	} while (_findnext(handle, &fd) != -1);

	_findclose(handle);
}

void VoxelComponent::LoadMapData(const char* _path)
{
	InitDataDirectory(_path);
	LoadMapInfo();
	NewChunks(info.width, info.height, info.depth);
	char totalPath[256];
	sprintf(totalPath,"%s\\chunks\\*.*", dataPath);
	struct _finddata_t fd;

	intptr_t handle;
	std::vector<XMFLOAT3> cList;
	if ((handle = _findfirst(totalPath, &fd)) == -1L)
		printf("NO FIle\n");
	do
	{
		char* token = NULL;
		char str[] = "_";
		token = strtok(fd.name, str);
		int p[3] = { 0,0,0 };
		int i = 0;
		while (token != NULL)
		{
			p[i++] = std::atoi(token);
			token = strtok(NULL, str);
		}
		cList.push_back(XMFLOAT3(p[0] * info.partitionSize, p[1] * info.partitionSize, p[2] * info.partitionSize));
	} while (_findnext(handle, &fd) == 0);

	_findclose(handle);

	for(auto i:cList)
		if(ReadVoxelData(i, dataPath))
			ReserveUpdate(i, Reserve_Load, false);



	//for (int x=0;x<info.width;x+=info.partitionSize)
	//{
	//	for (int y = 0; y < info.height; y += info.partitionSize)
	//	{
	//		for (int z = 0; z < info.depth; z += info.partitionSize)
	//		{
	//			if(ReadVoxelData(XMFLOAT3(x, y, z), dataPath))
	//				ReserveUpdate(XMFLOAT3(x, y, z), false, false);
	//		}
	//	}
	//}
}
void VoxelComponent::SaveMapData(const char* _path)
{
	InitDataDirectory(_path);
	SaveMapInfo();
	std::vector < OctreeNode<ChunkData>*> vec;
	tempChunks->root->GetLeafs(vec);
	SaveTask task;/////Task

	if (vec.size())
	{
		for (auto i : vec)
		{
			if (i->GetValue().IsHaveChunk()||i->GetValue().renderer)
			{
				task.commandBuffers.push_back(i->GetPosition());/////Task
				//WriteVoxelData(i->GetPosition());
			}
		}
	}
	task.component = this;/////Task
	task.Schedule(vec.size(), 2);/////Task
	task.Dispatch();/////Task
}
void VoxelComponent::LoadMapInfo()
{
	FILE* pInput = NULL;
	char totalPath[256];
	sprintf(totalPath, "%s\\mapInfo.JHCI", dataPath);
	fopen_s(&pInput, totalPath, "rb");
	if (pInput == NULL)
		return;
	fread(&info, sizeof(MapInfo), 1, pInput);
	fclose(pInput);
}
void VoxelComponent::InitDataDirectory(const char* _path)
{
	_mkdir(pathRoot);
	sprintf(dataPath, "%s\\%s",pathRoot, _path);
	_mkdir(dataPath);
	char path[256];
	sprintf(path, "%s\\chunks", dataPath);
	_mkdir(path);
}
void VoxelComponent::SaveMapInfo()
{
	FILE* pInput = NULL;
	char totalPath[256];
	sprintf(totalPath, "%s/mapInfo.JHCI", dataPath);
	fopen_s(&pInput, totalPath, "wb");
	if (pInput == NULL)
		return;
	fwrite(&info, sizeof(MapInfo), 1, pInput);
	fclose(pInput);
}


bool VoxelComponent::ReadVoxelData(XMFLOAT3 pos, const char* _path)
{
	bool isRendered = false;
	pos = GetPartitionStartPos(pos);
	auto node = tempChunks->GetNodeAtPosition(pos, 0);
	FILE* pInput = NULL;
	char totalPath[256];
	float ps = 1.0f / info.partitionSize;
	int x = (float)pos.x*ps;
	int y = (float)pos.y*ps;
	int z = (float)pos.z*ps;
	sprintf(totalPath, "%s\\chunks\\%d_%d_%d.JHCD", _path, x, y, z);
	fopen_s(&pInput, totalPath, "rb");
	if (pInput == NULL)
		return isRendered;
	int aNums = info.partitionSize * info.partitionSize*info.partitionSize;
	int elementSize = sizeof(VoxelData);
	if(node->GetDepth()!=0)
		node=tempChunks->Insert(pos, ChunkData(), 0);
	fread(&(node->GetValue().chunk[0]), elementSize, aNums, pInput);
	fread(&isRendered, sizeof(bool), 1, pInput);
	fclose(pInput);
	return isRendered;
}
void VoxelComponent::WriteVoxelData(XMFLOAT3 pos)
{
	pos = GetPartitionStartPos(pos);
	auto node = tempChunks->GetNodeAtPosition(pos, 0);
	if (node->GetDepth() != 0&&node->GetValue().renderer==NULL)
		return;
	bool isRendered = (node->GetValue().renderer != NULL);
	FILE* pInput = NULL;
	char totalPath[256];
	float ps = 1.0f / info.partitionSize;
	int x = (float)pos.x*ps;
	int y = (float)pos.y*ps;
	int z = (float)pos.z*ps;
	sprintf(totalPath, "%s\\chunks\\%d_%d_%d.JHCD", dataPath, x,y,z);
	fopen_s(&pInput, totalPath, "wb");
	if (pInput == NULL)
		return;
	int aNums = info.partitionSize * info.partitionSize*info.partitionSize;
	int elementSize = sizeof(VoxelData);
	fwrite(&(node->GetValue().chunk[0]), elementSize, aNums, pInput);
	fwrite(&isRendered, sizeof(bool), 1, pInput);
	fclose(pInput);
}


void VoxelComponent::LoadCube(int _width, int _height, int _depth)
{
	NewChunks(_width, _height, _depth);
	for (int x = 1; x < info.width; x++)
	{
		for (int z = 1; z < info.depth; z++)
		{
			for (int y = 1; y < info.height; y++)
			{
				VoxelData v;
				v.material = 1;
				v.isoValue = info.height-y-1;
				SetVoxel(x, y, z, v);
				if(x%info.partitionSize==0&&y%info.partitionSize==0&&z%info.partitionSize==0)
					ReserveUpdate(XMFLOAT3(x, y, z), Reserve_Load, false);
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
	NewChunks(_width,_height,_depth);
	for (int x = 0; x < info.width; x++)
	{
		for (int z = 0; z < info.depth; z++)
		{
			float noise = perlin.noise((float)x*refinement, (float)z*refinement, 0);
			noise *= (float)_maxHeight;
			for (int y = 0; y < _maxHeight; y++)
			{
				VoxelData vox;
				if (y <= noise)
				{
					vox.material = 1;
				}
				else
				{
				}
				vox.isoValue = noise - y;
				SetVoxel(x, y, z, vox,true);
				if ((x%info.partitionSize == 0) && (y%info.partitionSize == 0) && (z%info.partitionSize == 0))
					ReserveUpdate(XMFLOAT3(x, y, z), Reserve_Load,false);
			}
		}
	}
	printf("Load Perlin time : %dms \n", GetTickCount64() - tick);
}


void VoxelComponent::NewChunks(int _w, int _h, int _d)
{
	info.width = _w;
	info.height = _h;
	info.depth = _d;
	tempChunks = new Octree<ChunkData>(_w,info.partitionSize);
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

XMFLOAT3 VoxelComponent::GetPartitionCenter(XMFLOAT3 basePos)
{
	float hSize = info.partitionSize * 0.5f;
	return GetPartitionStartPos(basePos) + XMFLOAT3(hSize, hSize, hSize);
}
XMFLOAT3 VoxelComponent::GetPartitionStartPos(XMFLOAT3 basePos)
{
	float x, y, z;
	float size = 1 / (float)info.partitionSize;
	x = floor((basePos.x *size));
	y = floor((basePos.y *size));
	z = floor((basePos.z *size));
	return XMFLOAT3(x, y, z)*info.partitionSize;
}

int VoxelComponent::GetLODLevel(const XMFLOAT3& basePos, const XMFLOAT3& targetPos)
{
	if (targetPos.x < 0 || targetPos.y < 0 || targetPos.z < 0 || targetPos.x >= info.width || targetPos.y >= info.height || targetPos.z >= info.depth)
		return maxLODLevel;

	XMFLOAT3 basePos2 = GetPartitionCenter(basePos);
	XMFLOAT3 targetPos2 = GetPartitionCenter(targetPos);
	XMFLOAT3 n = targetPos2 - basePos2;
	XMFLOAT3 max, min;
	for (int i = 0; i <= maxLODLevel; i++)
	{
		//max = basePos2 + XMFLOAT3(LODDistance[i], LODDistance[i], LODDistance[i]);
		//min = basePos2 - XMFLOAT3(LODDistance[i], LODDistance[i], LODDistance[i]);
		//if (targetPos2.x <= max.x&&targetPos2.y <= max.y&&targetPos2.z <= max.z)
		//{
		//	if (targetPos2.x >= min.x&&targetPos2.y >= min.y&&targetPos2.z >= min.z)
		//	{
		//		return i;
		//	}
		//}
		if (abs(n.x) < LODDistance[i] 
			&& abs(n.y) < LODDistance[i]
			&& abs(n.z) < LODDistance[i])
		{
			return i;
		}
	}
	//XMVECTOR v1, v2;
	//float dis;
	//XMFLOAT3 basePos2 = GetPartitionCenter(basePos);
	//XMFLOAT3 targetPos2 = GetPartitionCenter(targetPos);
	//v1 = XMVectorSet(basePos2.x, basePos2.y, basePos2.z, 1);
	//v2 = XMVectorSet(targetPos2.x, targetPos2.y, targetPos2.z, 1);

	//dis = XMVectorGetX(XMVector3Length(XMVectorSubtract(v2, v1)));
	//for (int i = 0; i < 4; i++)
	//	if (LODDistance[i] >= dis)
	//	{
	//		return i;
	//	}
	return maxLODLevel;
}

void VoxelComponent::SetLODLevel(int level, float distance)
{
	if (level < 0 || level >= 8)
		return;
	distance -= (int)distance % info.partitionSize;
	LODDistance[level] = distance;
}

void VoxelComponent::ProcessCommandQueue()
{
	if (!useAsyncBuild)
	{
		for (int t = 0; t < 3; t++)
		{
			if (commandQueue[t].size())
			{
				//ULONG tick = GetTickCount64();
				for (auto i : commandQueue[t])
				{
					RESULT_BUFFER result = UpdatePartialMesh(XMFLOAT3(i.x, i.y, i.z), i.lodLevel, i.transitionCellBasis);
					UpdateMeshRenderer(result.newMesh, result.pos, result.lodLevel);
				}
				commandQueue[t].clear();
				//printf("updateQueue_Main Generated %dms\n", GetTickCount64() - tick);
			}
		}
	}
	else
	{
		if(!commandQueue[0].size()&& !commandQueue[1].size()&&!commandQueue[2].size())
		{ 
			return;
		}
		//clock_t time = clock();
		int length = 16, batch = 1;
		int handle = -1;
		PolygonizeTask job[3];
		
		for (int t = 0; t < 3; t++)
		{
			job[t].component = this;
			for (int i = 0; i < length; i++)
			{
				if (!commandQueue[t].size())
					break;
				job[t].commandBuffers.push_back(commandQueue[t].front());
				commandQueue[t].pop_front();
			}
			job[t].resultBuffers.resize(job[t].commandBuffers.size());
			handle = job[t].Schedule(length, batch, handle);
		}
		job[2].Dispatch();
		for (int t = 0; t < 3; t++)
		{
			for (auto i : job[t].resultBuffers)
				UpdateMeshRenderer(i.newMesh, i.pos, i.lodLevel);
		}
		//printf("%d ms\n", clock() - time);		
		//while (commandQueue_Main.size())
		//{
		//	COMMAND_BUFFER _node = commandQueue_Main.front();
		//	commandQueue_Main.pop_front();
		//	threadPool_Main.AddTask(_node);
		//}
		//while (commandQueue_Deform.size())
		//{
		//	COMMAND_BUFFER _node = commandQueue_Deform.front();
		//	commandQueue_Deform.pop_front();
		//	threadPool_Deform.AddTask(_node);
		//}
	}
}

void VoxelComponent::ProcessResultQueue()
{
	if (threadPool_Main.IsInitialized())
	{
		if (threadPool_Main.GetResultQueue()->size())
		{
			//ULONG tick = GetTickCount64();
			for (auto i : *(threadPool_Main.GetResultQueue()))
			{
				UpdateMeshRenderer(i.newMesh, i.pos,i.lodLevel);
			}
			threadPool_Main.GetResultQueue()->clear();
			//printf("threadPool_Main Updated %dms\n", GetTickCount64() - tick);
		}
	}
	if (threadPool_Deform.IsInitialized())
	{
		if (threadPool_Deform.GetResultQueue()->size())
		{
			//ULONG tick = GetTickCount64();
			for (auto i : *(threadPool_Deform.GetResultQueue()))
			{
				UpdateMeshRenderer(i.newMesh, i.pos,i.lodLevel);
			}
			threadPool_Deform.GetResultQueue()->clear();
			//printf("threadPool_Deform Updated %dms\n", GetTickCount64() - tick);
		}
	}
}
short VoxelComponent::GetTransitionBasis(int lodLevel, XMFLOAT3 basePos, XMFLOAT3 targetPos)
{
	if (lodLevel == 0)
		return 0;
	short transitionCellBasis = 0;
	XMFLOAT3 r(info.partitionSize, 0, 0), u(0, info.partitionSize, 0), f(0, 0, info.partitionSize);
	int l[6] = { GetLODLevel(basePos, targetPos - f),GetLODLevel(basePos, targetPos + f),GetLODLevel(basePos, targetPos - r),GetLODLevel(basePos, targetPos + r),GetLODLevel(basePos, targetPos - u),GetLODLevel(basePos, targetPos + u) };
	for (int i = 0; i < 6; i++)
		if (l[i] >= 0 && l[i] < lodLevel)
			transitionCellBasis |= (1 << i);
	return transitionCellBasis;
}
short VoxelComponent::ReserveUpdate(XMFLOAT3 pos, short _basis, short _lodLevel, ReserveType _reserveType, bool isEnableOverWrite)
{
	if (pos.x >= info.width || pos.y >= info.height || pos.z >= info.depth)
		return -1;
	if (pos.x < 0 || pos.y < 0 || pos.z < 0)
		return -1;
	COMMAND_BUFFER input;
	XMFLOAT3 targetPos = GetPartitionStartPos(pos);
	input.x = (int)targetPos.x;
	input.y = (int)targetPos.y;
	input.z = (int)targetPos.z;
	input.lodLevel = _lodLevel;
	input.transitionCellBasis = _basis;
	if (isEnableOverWrite)
	{
		auto iter = std::find(commandQueue[_reserveType].begin(), commandQueue[_reserveType].end(), input);
		if (iter != commandQueue[_reserveType].end())
		{
			iter->lodLevel = input.lodLevel;
			iter->transitionCellBasis = input.transitionCellBasis;
			return input.lodLevel;
		}
	}
	commandQueue[_reserveType].push_back(input);
	return input.lodLevel;
}

short VoxelComponent::ReserveUpdate(XMFLOAT3 pos, ReserveType _reserveType,bool isEnableOverWrite)
{
	XMFLOAT3 basePos = GetLastBasePosition();
	float lodLevel = GetLODLevel(basePos, pos);
	short transitionCellBasis = GetTransitionBasis(lodLevel, basePos, pos);
	return ReserveUpdate(pos, transitionCellBasis, lodLevel, _reserveType, isEnableOverWrite);
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

bool PolygonizeTask::Excute(int index)
{
	if (commandBuffers.size() <= index)
		return false;
	auto i = commandBuffers[index];
	resultBuffers[index] = component->UpdatePartialMesh(XMFLOAT3(i.x, i.y, i.z), i.lodLevel, i.transitionCellBasis);
	return true;
}

bool SaveTask::Excute(int index)
{
	if (commandBuffers.size() <= index)
		return false;
	auto i = commandBuffers[index];
	component->WriteVoxelData(i);
	return true;
}
