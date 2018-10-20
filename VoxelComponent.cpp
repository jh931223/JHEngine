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


#include"VoxelTable.h"


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

	info.partitionSize = 16;
	//SetLODLevel(0, 100000);


	//SetLODLevel(0, 64);
	//SetLODLevel(1, 128);
	//SetLODLevel(2, 256);
	//SetLODLevel(3, 256);

	int start = 128;

	SetLODLevel(0, start);
	SetLODLevel(1, start + info.partitionSize);
	SetLODLevel(2, start + info.partitionSize + info.partitionSize);
	SetLODLevel(3, start + info.partitionSize + info.partitionSize+ info.partitionSize);
	SetLODLevel(4, start + info.partitionSize + info.partitionSize + info.partitionSize+ info.partitionSize);
	//SetLODLevel(3, 256);


	//lastBasePosition = XMFLOAT3(0, 30, 0);
	lastBasePosition = XMFLOAT3(3000,3000,3000);
	isLockedBasePosition = true;
	//customPos = lastBasePosition; 


	//LoadCube(32, 32, 32);
	LoadPerlin(256, 128, 256,128, 0.2f);
	//LoadPerlin(2048, 256, 2048, 128, 0.07f);
	//LoadMapData("Terrain1");
	//int h = ReadTXT("/data/info.height.txt");
	//LoadHeightMapFromRaw(1025, 256, 1025,256, "data/terrain.raw");// , 0, 0, 255, 255);


#ifndef USE_JOBSYSTEM
	std::function<RESULT_BUFFER(COMMAND_BUFFER)> _task = ([&, this](COMMAND_BUFFER buf)
	{

		return this->UpdatePartialMesh(XMFLOAT3(buf.x, buf.y, buf.z), buf.lodLevel, buf.transitionCellBasis);
	});
	threadPool[Reserve_Load].SetTaskFunc(_task);
	threadPool[Reserve_Load].Initialize(8, false);
	threadPool[Reserve_Deform].SetTaskFunc(_task);
	threadPool[Reserve_Deform].Initialize(4, false);
	threadPool[Reserve_LOD].SetTaskFunc(_task);
	threadPool[Reserve_LOD].Initialize(8, false);
#endif




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

	auto chunk = meshRendererOctree->Subdivide(pos, 0, newMesh!=NULL);
	if (newMesh)
	{
		if (chunk->GetValue() == NULL)
		{
			GameObject* gobj = new GameObject("voxel partition");
			MeshRenderer* newRenderer = new MeshRenderer;
			//newRenderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_triplanar"));
			newRenderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_triplanar"));
			gobj->AddComponent(newRenderer);
			if(chunk->GetDepth())
				chunk = meshRendererOctree->Insert(pos, newRenderer, 0);
			else chunk->SetValue(newRenderer);
			float hSize = (float)info.partitionSize*0.5f;
			newRenderer->boundary.centerOffset = chunk->GetPosition()+XMFLOAT3(hSize, hSize, hSize);
			newRenderer->boundary.size = hSize;
		}
		else if (chunk->GetValue()->GetMesh())
		{
			chunk->GetValue()->ReleaseMesh();
		}
		chunk->GetValue()->SetMesh(newMesh);
	}
	else
	{
		if (chunk)
		{
			if (chunk->GetValue())
			{
				if (chunk->GetValue()->GetMesh())
				{
					chunk->GetValue()->ReleaseMesh();
				}
				GameObject::Destroy(chunk->GetValue()->gameObject);
				chunk->SetValue(NULL);
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
		LightComponent::mainLight()->transform()->SetPosition(CameraComponent::mainCamera()->transform()->GetWorldPosition());
		LightComponent::mainLight()->GenerateViewMatrix();
	}
	else if (Input()->GetKeyDown(DIK_F4))
	{
		useAsyncBuild = !useAsyncBuild;
	}
	if (Input()->GetKeyDown(DIK_R))
	{
		isLockedBasePosition = true;
		lastBasePosition = GetPartitionStartPos(camera->transform->GetWorldPosition());
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
	if (Input()->GetKeyDown(DIK_K))
	{
		Frustum::isLockFrustum = !Frustum::isLockFrustum;
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
		EditVoxel(pos, brushRadius, -strength);
	}
	else if (Input()->GetKey(VK_LBUTTON))
	{
		XMFLOAT3 pos = camera->transform->GetWorldPosition() + camera->transform->forward() *brushRadius;
		XMFLOAT3 cpos = CovertToChunkPos(pos, false);
		EditVoxel(pos, brushRadius, strength);
	}
	ProcessCommandQueue();
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
	if (meshRendererOctree)
		delete meshRendererOctree;
	meshRendererOctree = 0;
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

void VoxelComponent::PolygonizeRegularCell(XMFLOAT3 pos, XMINT3 offset, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, RegularCellCache* cache, XMFLOAT3 min, XMFLOAT3 max)
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
	return PolygonizeRegularCell(pos, offset,_unit, vertices, indices,cache, newCorners);
}
void VoxelComponent::PolygonizeRegularCell(XMFLOAT3 pos,XMINT3 offset, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, RegularCellCache* cache, XMFLOAT3 newCorners[])
{
	XMFLOAT3 totalPos = pos + XMFLOAT3(offset.x, offset.y, offset.z)*_unit;
	if (totalPos.x + _unit >= info.width || totalPos.y + _unit >= info.height || totalPos.z + _unit >= info.depth)
		return;

	float density[8] = { -1,-1,-1,-1,-1,-1,-1,-1 };
	int caseCode = 0;
	for (int i = 0; i < 8; i++)
	{
		density[i] = GetVoxel(totalPos + regularCorners[i] * _unit).isoValue;
		newCorners[i] = totalPos + newCorners[i] * _unit;
		if (density[i] < isoLevel)
			caseCode |= 1 << i;
	}
	unsigned char cellClass = regularCellClass[caseCode];
	if (cellClass == 0)
		return;
	int directionMask = ((offset.x >= 0) ? 1 : 0) | ((offset.y >= 0) ? 4 : 0) | ((offset.z >= 0) ? 2 : 0);
	RegularCell regularCell = regularCellData[cellClass];
	int vertCount = regularCell.GetVertexCount();
	int triCount = regularCell.GetTriangleCount();
	std::vector<unsigned short> vertDatas;
	for (int i = 0; i < vertCount; i++)
		vertDatas.push_back(regularVertexData[caseCode][i]);
	int vertBegin = vertices.size();
	std::vector<int> localIndices;
	for (int i = 0; i < vertCount; i++)
	{
		int d = vertDatas[i] & 0xFF;
		int d0 = d >> 4;
		int d1 = d & 0xF;
		int reuse = vertDatas[i] >> 8;
		int dir = (reuse >> 4)&0xF;
		int idx = (reuse) & 0xF;
		bool present = (dir&directionMask)==dir;
		int newIndex = -1;
		if (cache&&present&&d1 != 7)
		{
			auto prevCache = cache->GetReusedCell(offset, dir);
			if(prevCache)
				newIndex = prevCache->verts[idx];
		}
		if(newIndex ==-1)
		{
			XMFLOAT3 p0, p1;
			p0 = newCorners[d0];
			p1 = newCorners[d1];
			float mu = (isoLevel - density[d0]) / (density[d1] - density[d0]);
			VertexBuffer newVertex;
			newVertex.position = (lerpSelf(p0, p1, mu));
			newVertex.normal = XMFLOAT3(0, 0, 0);
			newVertex.color = XMFLOAT4();
			vertices.push_back(newVertex);
			newIndex = vertices.size()-1;
		}
		if (cache&&(dir & 8) != 0)
		{
			(*cache)[offset]->verts[idx] = newIndex;
		}
		localIndices.push_back(newIndex);
	}
	int minX = pos.x;
	int maxX = pos.x + info.partitionSize-_unit;
	//int minY = pos.x + offset.x*_unit;
	//int maxY = pos.x + offset.x*_unit;
	int minZ = pos.z;
	int maxZ = pos.z + info.partitionSize - _unit;
	for (int i = 0; i < triCount; i++)
	{
		int triBegin = i * 3;
		int index_0 = localIndices[regularCell.Indices()[triBegin]];
		int index_1 = localIndices[regularCell.Indices()[triBegin + 1]];
		int index_2 = localIndices[regularCell.Indices()[triBegin + 2]];
		XMFLOAT3 n = CalcNormal(vertices[index_0].position, vertices[index_1].position, vertices[index_2].position);
		//float ndotu = Dot(n, BasicVector::up);
		//if(ndotu)
		//XMFLOAT3 nUp(0, ndotu >= 0?1:-1, 0);
		//std::function<bool(int)> normalCheck = [&](int _index) {

		//	if (vertices[_index].position.x == minX || vertices[_index].position.x == maxX)
		//		return false;
		//	if (vertices[_index].position.z == minZ || vertices[_index].position.z == maxZ)
		//		return false;
		//	return true;
		//};
		//if(!normalCheck(index_0))
			vertices[index_0].normal += n;
		//vertices[index_0].normal += nUp;
		//if (!normalCheck(index_1))
			vertices[index_1].normal += n;
		//vertices[index_1].normal += nUp;
		//if (!normalCheck(index_2))
			vertices[index_2].normal += n;
		//vertices[index_2].normal += nUp;
		indices.push_back(index_0);
		indices.push_back(index_1);
		indices.push_back(index_2);
	}
}
void VoxelComponent::PolygonizeTransitionCell(XMFLOAT3 pos,XMINT3 offset, int _unit, short _basis, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
{
	XMFLOAT3 totalPos = pos + XMFLOAT3(offset.x, offset.y, offset.z)*_unit;
	if (totalPos.x + _unit >= info.width || totalPos.y + _unit >= info.height || totalPos.z + _unit >= info.depth)
		return;
	int caseCode = 0;
	std::vector<XMFLOAT3> newCorners;
	XMFLOAT3 startPos = GetPartitionStartPos(pos);
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

	PolygonizeRegularCell(pos, offset,_unit, vertices, indices, NULL, vertOut);
	//PolygonizeRegularCell(pos, _unit, vertices, indices, innerBoxMin, innerBoxMax);
	int basis = _basis;
	for(int b=0;b<6;b++)
	{
		if (basis & (1 << b))
		{
			newCorners.clear();
			float density[13] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
			for (int j=0;j<9;j++)
			{
				newCorners.push_back(totalPos + transitionCornersByBasis[b][j] * _unit);
				density[j] = GetVoxel(newCorners[j]).isoValue;
			}
			for (int j = 0; j < 4; j++)
				newCorners.push_back(totalPos + _9abc[b][j] * _unit);

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

void VoxelComponent::GetVertexInnerBox(short _basis,XMINT3 offset,int _unit,XMFLOAT3 vertOut[])
{
	int max = info.partitionSize/_unit-1;
	vertOut[0] = XMFLOAT3(0, 0, 0);
	vertOut[1] = XMFLOAT3(1, 0, 0);
	vertOut[2] = XMFLOAT3(0, 0, 1);
	vertOut[3] = XMFLOAT3(1, 0, 1);
	vertOut[4] = XMFLOAT3(0, 1, 0);
	vertOut[5] = XMFLOAT3(1, 1, 0);
	vertOut[6] = XMFLOAT3(0, 1, 1);
	vertOut[7] = XMFLOAT3(1, 1, 1);

	float lowValue = 0.001f;
	float highValue = 0.999f;

	if (_basis & 1)
	{
		vertOut[0].z = lowValue;//��
		vertOut[1].z = lowValue;//��
		vertOut[4].z = lowValue;//��
		vertOut[5].z = lowValue;//��
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
		vertOut[2].z = highValue;//��
		vertOut[3].z = highValue;//��
		vertOut[6].z = highValue;//��
		vertOut[7].z = highValue;//��
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

RESULT_BUFFER VoxelComponent::GeneratePartitionFaces(XMFLOAT3 pos, int lodLevel, short transitionCellBasis)
{
	//ULONG time = GetTickCount();
	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	pos = GetPartitionStartPos(pos);
	float  _cellUnit=pow(2,lodLevel);
	int max = info.partitionSize - _cellUnit;
	
	RegularCellCache cache(info.partitionSize,_cellUnit);

	int size = info.partitionSize / _cellUnit;


	for (int i=0;i<size && ((pos.x + i*_cellUnit)<info.width);i++)
	{
		for (int j = 0; j < size && ((pos.y + j*_cellUnit)<info.height); j ++)
		{
			for (int k = 0; (k < size && ((pos.z + k * _cellUnit)<info.depth) ); k ++)
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
						if ((transitionCellBasis & 2) && k == size-1)
							basis |= 2;
						if ((transitionCellBasis & 4) && i == 0)
							basis |= 4;
						if ((transitionCellBasis & 8) && i == size - 1)
							basis |= 8;
						if ((transitionCellBasis & 16) && j == 0)basis |= 16;
						if ((transitionCellBasis & 32) && j == size - 1)basis |= 32;
					}
					if (lodLevel>0&&basis)
					{
						transitionCellBasis = transitionCellBasis;
						PolygonizeTransitionCell(pos, XMINT3(i, j, k), _cellUnit, basis, vertices, indices);
					}
					else
						PolygonizeRegularCell(pos, XMINT3(i, j, k), _cellUnit,vertices, indices, &cache);
				}
			}
		}
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
	return true;
}



bool VoxelComponent::EditVoxel(XMFLOAT3 pos, float _radius, float _strength,BrushType _brushType)
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
				float amount = _radius - dis;
				XMFLOAT3 nPos(pos.x + i - hUnit, pos.y + j - hUnit, pos.z + k - hUnit);
				VoxelData data = GetVoxel(nPos);
				float lastValue = data.isoValue;
				data.isoValue = amount*SIGN(_strength);
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
					XMFLOAT3 offset[7] = { XMFLOAT3(1,0,0),XMFLOAT3(0,1,0),XMFLOAT3(0,0,1),XMFLOAT3(1,1,0),XMFLOAT3(0,1,1),XMFLOAT3(1,1,1) };
					for (int o = 0; o < 7; o++)
					{
						XMFLOAT3 oPos = GetPartitionStartPos(nPos-offset[o]);
						if(pPos!=oPos)
							if(IsPolygonizableCell(nPos-offset[i]))
								ReserveUpdate(oPos, Reserve_Deform);
					}
				}
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
void VoxelComponent::RefreshLODNodes(XMFLOAT3 basePos)
{

	//clock_t t = clock();
	basePos = GetPartitionStartPos(basePos);
	std::unordered_map<int, LODGroupData> newLoadedLODGroup;
	std::unordered_map<int, LODGroupData> newLODGroup;
	std::vector<int> newLODGroupIndex[maxLODLevel + 1];
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
					newLODGroup[index] = lodGroupData;
					newLODGroupIndex[lodGroupData.level].push_back(index);
				}
				else
				{
					if (lodGroupData.transitionBasis == iter->second.transitionBasis&&lodGroupData.level == iter->second.level)
						newLoadedLODGroup[index] = lodGroupData;
					else
					{
						oldLODGroups.erase(iter);
						newLODGroup[index] = lodGroupData;
						newLODGroupIndex[lodGroupData.level].push_back(index);
					}
				}
			}
		}
	}

	for (auto i : newLoadedLODGroup)
	{
		lodGropups[i.first] = i.second;
	}
	for (int i = 0; i <= maxLODLevel; i++)
	{
		for (auto j : newLODGroupIndex[i])
		{
			LODGroupData data = newLODGroup[j];
			auto result=ReserveUpdate(GetPositionFromIndex(j), data.transitionBasis, data.level,
//#ifdef USE_THREADPOOL
//				Reserve_Load
//#else 
				Reserve_LOD
//#endif
				, true);
			lodGropups[j] = data;
		}
	}
	//for (auto i : newLODGroup)
	//{
	//	ReserveUpdate(GetPositionFromIndex(i.first),i.second.transitionBasis,i.second.level, Reserve_LOD, true);
	//	lodGropups[i.first] = i.second;
	//}
	for (auto i : oldLODGroups)
	{
		ReserveUpdate(GetPositionFromIndex(i.first),
//#ifdef USE_THREADPOOL
//			Reserve_Load
//#else 
			Reserve_LOD
//#endif
			, true);
	}
	oldLODGroups.clear();
	//printf("LOD Update %dms\n", clock() - t);

}

void VoxelComponent::ProcessLOD()
{
	if (isLockedBasePosition)
		return;
	XMFLOAT3 newPosition = GetPartitionStartPos(CameraComponent::mainCamera()->transform()->GetWorldPosition());
	float processDis = info.partitionSize * 2;
	float distance = GetDistance(newPosition, lastBasePosition);
	if (distance>=processDis)
	{
		lastBasePosition = newPosition;
		RefreshLODNodes(newPosition);
	}
	return;
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



	printf("%d BYTE chunk data ���� �Ϸ�\n", info.width*info.depth*info.height);
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

bool VoxelComponent::IsPolygonizableCell(XMFLOAT3 pos, int _size)
{
	XMFLOAT3 offset[8] = { XMFLOAT3(),XMFLOAT3(_size,0,0),XMFLOAT3(0,_size,0),XMFLOAT3(0,0,_size),XMFLOAT3(_size,_size,0),XMFLOAT3(0,_size,_size),XMFLOAT3(_size,_size,_size) };
	for (int i = 0; i < 8; i++)
		if (GetVoxel(pos + offset[i]).isoValue > isoLevel)
			return true;
	return false;
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
			if (i->GetValue().IsHaveChunk()|| meshRendererOctree->GetNodeAtPosition(i->GetPosition(),0)->GetValue())
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
	auto node2 = meshRendererOctree->GetNodeAtPosition(pos, 0);
	if (node->GetDepth() != 0&& node2->GetValue()==NULL)
		return;
	bool isRendered = (node2->GetValue() != NULL);
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
	meshRendererOctree = new Octree<MeshRenderer*>(_w, info.partitionSize);
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

	//XMFLOAT3 basePos2 = GetPartitionCenter(basePos);
	//XMFLOAT3 targetPos2 = GetPartitionCenter(targetPos);
	//XMFLOAT3 n = targetPos2 - basePos2;
	//XMFLOAT3 max, min;
	//for (int i = 0; i <= maxLODLevel; i++)
	//{
	//	//max = basePos2 + XMFLOAT3(LODDistance[i], LODDistance[i], LODDistance[i]);
	//	//min = basePos2 - XMFLOAT3(LODDistance[i], LODDistance[i], LODDistance[i]);
	//	//if (targetPos2.x <= max.x&&targetPos2.y <= max.y&&targetPos2.z <= max.z)
	//	//{
	//	//	if (targetPos2.x >= min.x&&targetPos2.y >= min.y&&targetPos2.z >= min.z)
	//	//	{
	//	//		return i;
	//	//	}
	//	//}
	//	if (abs(n.x) < LODDistance[i] 
	//		&& abs(n.y) < LODDistance[i]
	//		&& abs(n.z) < LODDistance[i])
	//	{
	//		return i;
	//	}
	//}
	XMVECTOR v1, v2;
	float dis;
	XMFLOAT3 basePos2 = GetPartitionCenter(basePos);
	XMFLOAT3 targetPos2 = GetPartitionCenter(targetPos);
	v1 = XMVectorSet(basePos2.x, basePos2.y, basePos2.z, 1);
	v2 = XMVectorSet(targetPos2.x, targetPos2.y, targetPos2.z, 1);

	dis = XMVectorGetX(XMVector3Length(XMVectorSubtract(v2, v1)));
	for (int i = 0; i <= maxLODLevel; i++)
		if (LODDistance[i] > dis)
		{
			return i;
		}
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
		

#ifndef USE_JOBSYSTEM
		for (int i = 0; i < 3; i++)
		{
			while (commandQueue[i].size())
			{
				threadPool[i].AddTaskList(commandQueue[i]);
				commandQueue[i].clear();
				//COMMAND_BUFFER _node = commandQueue[i].front();
				//commandQueue[i].pop_front();
				//threadPool[i].AddTask(_node);
			}
		}
#else
		int length = 16, batch = 1;
		int handle = -1;
		int lastJob = -1;
		PolygonizeTask job[3];

		for (int t = 0; t < 3; t++)
		{
		if (commandQueue[t].size())
		{
		job[t].component = this;
		int _l = (t == Reserve_LOD) ? 4 : length;
		for (int i = 0; i < _l; i++)
		{
		if (!commandQueue[t].size())
		break;
		job[t].commandBuffers.push_back(commandQueue[t].front());
		commandQueue[t].pop_front();
		}
		job[t].resultBuffers.resize(job[t].commandBuffers.size());
		handle = job[t].Schedule(_l, batch, handle);
		lastJob = t;
		}
		}
		if (lastJob == -1)
		return;
		job[lastJob].Dispatch();
		for (int t = 0; t < 3; t++)
		{
		for (auto i : job[t].resultBuffers)
		UpdateMeshRenderer(i.newMesh, i.pos, i.lodLevel);
		}
#endif
	}
}

void VoxelComponent::ProcessResultQueue()
{
#ifndef USE_JOBSYSTEM
	for (int i = 0; i < 3; i++)
	{
		if (threadPool[i].IsInitialized())
		{
			std::list<RESULT_BUFFER> resultQueue;
			threadPool[i].GetResultQueue(resultQueue);
			if (resultQueue.size())
			{
				//ULONG tick = GetTickCount64();
				for (auto i : resultQueue)
				{
					UpdateMeshRenderer(i.newMesh, i.pos, i.lodLevel);
				}
				resultQueue.clear();
				//printf("threadPool_Main Updated %dms\n", GetTickCount64() - tick);
			}
		}
	}
#endif
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
COMMAND_BUFFER VoxelComponent::ReserveUpdate(XMFLOAT3 pos, short _basis, short _lodLevel, ReserveType _reserveType, bool isEnableOverWrite, bool autoPush)
{
	COMMAND_BUFFER input;
	if (pos.x >= info.width || pos.y >= info.height || pos.z >= info.depth)
		return input;
	if (pos.x < 0 || pos.y < 0 || pos.z < 0)
		return input;
	XMFLOAT3 targetPos = GetPartitionStartPos(pos);
	input.x = (int)targetPos.x;
	input.y = (int)targetPos.y;
	input.z = (int)targetPos.z;
	input.lodLevel = _lodLevel;
	input.transitionCellBasis = _basis;
	if (isEnableOverWrite&&autoPush)
	{
		auto iter = std::find(commandQueue[_reserveType].begin(), commandQueue[_reserveType].end(), input);
		if (iter != commandQueue[_reserveType].end())
		{
			iter->lodLevel = input.lodLevel;
			iter->transitionCellBasis = input.transitionCellBasis;
			return input;
		}
	}
	auto node = tempChunks->GetNodeAtPosition(targetPos, 0);
	auto node2 = meshRendererOctree->GetNodeAtPosition(targetPos, 0);
	if (node->GetDepth() != 0&& node2->GetValue()==NULL)
	{
		int size = info.partitionSize;
		XMFLOAT3 offset[7] = { XMFLOAT3(size,0,0),XMFLOAT3(0,size,0),XMFLOAT3(0,0,size),XMFLOAT3(size,size,0),XMFLOAT3(size,0,size),XMFLOAT3(0,size,size),XMFLOAT3(size,size,size) };
		bool reserve = false;
		for (int i = 0; i < 7; i++)
		{
			auto node_extra = tempChunks->GetNodeAtPosition(targetPos + offset[i], 0);
			if (node_extra->GetValue().IsHaveChunk())
			{
				reserve = true;
				break;
			}
		}
		if (!reserve)
			return COMMAND_BUFFER();
	}
	if(autoPush)
		commandQueue[_reserveType].push_back(input);
	return input;
}

COMMAND_BUFFER VoxelComponent::ReserveUpdate(XMFLOAT3 pos, ReserveType _reserveType,bool isEnableOverWrite, bool autoPush)
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
