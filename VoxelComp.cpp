#pragma once
#include"stdafx.h"
#include"VoxelComp.h"
#include"MeshClass.h"
#include"MeshRenderer.h"
#include"SystemClass.h"
#include"TextureClass.h"
#include"CameraComponent.h"
#include"LightComponent.h"
#include"Octree.h"
#include"ArrayedOctree.h"

#include<vector>
#include<map>
#include<algorithm>
#include<list>
#include<unordered_map>
#include<unordered_set>
#include<time.h>
#include"PerlinNoise.h"
#include<amp.h>
#include<amp_math.h>
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
#include"VoxelTable.h"
using namespace concurrency;
using namespace concurrency::graphics;
using namespace concurrency::direct3d;
using namespace concurrency::fast_math;

VoxelComp::VoxelComp()
{
}

VoxelComp ::~VoxelComp()
{
	if(chunks)
		delete[] chunks;
	chunks = 0;
	if (aOctree)
		delete aOctree;
	aOctree = 0;
	ReleaseOctree();
}
void VoxelComp::Initialize()
{

	camera = Hierarchy()->FindGameObjectWithName("mainCamera");


	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;

	//useFrustum = true;

	partitionSize = 32;
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
	//LoadPerlin(256, 128, 256,128, 0.1f);
	LoadPerlin(1024, 128, 1024, 1, 0.1f);
	//int h = ReadTXT("/data/height.txt");
	//LoadHeightMapFromRaw(1024, 512, 1024,256, "data/terrain.raw");// , 0, 0, 255, 255);

	std::function<MESH_RESULT(INPUT_BUFFER)> _task = ([&, this](INPUT_BUFFER buf)
	{
		//float halfSize = this->GetPartitionSize()*0.5f;
		//XMFLOAT3 targetPos = XMFLOAT3(buf.x + halfSize, buf.y + halfSize, buf.z + halfSize);
		return this->UpdatePartialMesh(XMFLOAT3(buf.x, buf.y, buf.z), buf.lodLevel,buf.transitionCellBasis);
	});
	threadPool_Main.SetTaskFunc(_task);
	threadPool_Deform.SetTaskFunc(_task);
	threadPool_Main.Initialize(8, false);
	threadPool_Deform.Initialize(8, false);





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
	else if (Input()->GetKeyDown(DIK_F3))
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
void VoxelComp::PolygonizeCell(XMFLOAT3 pos, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, XMFLOAT3 min, XMFLOAT3 max)
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
void VoxelComp::PolygonizeCell(XMFLOAT3 pos, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, XMFLOAT3 newCorners[])
{
	if (pos.x + _unit >= width || pos.y + _unit >= height || pos.z + _unit >= depth)
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
	std::vector<std::vector<XMFLOAT3>> vertNormalBuffer;
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
		vertNormalBuffer.push_back(std::vector<XMFLOAT3>());
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




void VoxelComp::GetVertexInnerBox(short _basis,XMFLOAT3 offset,int _unit,XMFLOAT3 vertOut[])
{
	int max = partitionSize - _unit;
	vertOut[0] = XMFLOAT3(0, 0, 0);
	vertOut[1] = XMFLOAT3(1, 0, 0);
	vertOut[2] = XMFLOAT3(0, 0, 1);
	vertOut[3] = XMFLOAT3(1, 0, 1);
	vertOut[4] = XMFLOAT3(0, 1, 0);
	vertOut[5] = XMFLOAT3(1, 1, 0);
	vertOut[6] = XMFLOAT3(0, 1, 1);
	vertOut[7] = XMFLOAT3(1, 1, 1);

	float lowValue = 0.3f;
	float highValue = 0.7f;

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
void VoxelComp::PolygonizeTransitionCell(XMFLOAT3 pos, int _unit, short _basis, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
{
	if (pos.x + _unit >= width || pos.y + _unit >= height || pos.z + _unit >= depth)
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

VoxelComp::MESH_RESULT VoxelComp::GeneratePartitionFaces(XMFLOAT3 pos, int lodLevel, short transitionCellBasis)
{
	//ULONG time = GetTickCount();
	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	pos = GetPartitionStartPos(pos);
	float  _cellUnit=pow(2,lodLevel);
	int max = partitionSize - _cellUnit;
	for (int i=0;i<partitionSize && (pos.x + i<width);i+= _cellUnit)
	{
		for (int j = 0; j < partitionSize && (pos.y + j<height); j += _cellUnit)
		{
			for (int k = 0; k < partitionSize&&(pos.z + k<depth); k += _cellUnit)
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
	//printf("Marching Cube Create Buffer ( CPU ): %d ms x:%f y:%f z:%f\n", GetTickCount() - time,pos.x,pos.y,pos.z);
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

XMFLOAT3 VoxelComp::CalcNormal(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3)
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

XMFLOAT2 VoxelComp::GetUV(BYTE type)
{
	if (type == -1)
		return XMFLOAT2(0, 0);
	return XMFLOAT2(tUnit*(type - 1), (int)(type / tAmount) * tUnit);
}

VoxelComp::VoxelData VoxelComp::GetVoxel(int x, int y, int z)
{
	VoxelData v;
	if (x >= width || y >= height || z >= depth)
		return v;
	if (x < 0 || y < 0 || z < 0)
		return v;
	int index = GetCellIndex(x, y, z);
	if (useArrayedOctree)
		v = aOctree->GetNode(XMFLOAT3(x, y, z), 0);
	else if (!useSpartialMatrix)
		v = chunks[index];
	else
	{
		auto iter = spartialMatrix.find(index);
		if (iter != spartialMatrix.end())
			v = iter->second;
	}
	return v;
}

VoxelComp::VoxelData VoxelComp::GetVoxel(XMFLOAT3 pos)
{
	return GetVoxel((int)pos.x, (int)pos.y, (int)pos.z);
}

unsigned int VoxelComp::GetCellIndex(int x, int y, int z)
{
	return x + y * width + z * width*height;
}
bool VoxelComp::SetVoxel(int x, int y, int z, VoxelData value,bool isInit)
{
	if (x >= width || y >= height || z >= depth)
		return false;
	if (x < 0 || y < 0 || z < 0)
		return false;
	XMFLOAT3 pos(x, y, z);
	VoxelData vox = value;
	int index = GetCellIndex(x, y, z);
	if (useArrayedOctree)
		aOctree->SetNode(pos,value);
	else if(!useSpartialMatrix)
		chunks[index] = value;
	else
	{
		if (!isInit)
			spartialMatrix[index] = value;
		else
			spartialMatrix.insert_or_assign(index, value);
	}
	return true;
}

bool VoxelComp::EditVoxel(XMFLOAT3 pos, float _radius, float _strength)
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

						ReserveUpdate(nPos, true);
						XMFLOAT3 pPos = GetPartitionStartPos(nPos);
						if(pPos.x == nPos.x)
							ReserveUpdate(nPos + XMFLOAT3(-1, 0, 0),true);
						if (pPos.y == nPos.y)
							ReserveUpdate(nPos + XMFLOAT3(0, -1, 0), true);
						if (pPos.x == nPos.x&&pPos.y == nPos.y)
							ReserveUpdate(nPos + XMFLOAT3(-1, -1, 0), true);
						if (pPos.x == nPos.x&&pPos.y == nPos.y&&pPos.z==nPos.z)
							ReserveUpdate(nPos + XMFLOAT3(-1, -1, -1), true);
						if (pPos.y == nPos.y&&pPos.z == nPos.z)
							ReserveUpdate(nPos + XMFLOAT3(0, -1, -1), true);
						if (pPos.x == nPos.x&&pPos.z == nPos.z)
							ReserveUpdate(nPos + XMFLOAT3(-1, 0, -1), true);
						if (pPos.z == nPos.z)
							ReserveUpdate(nPos + XMFLOAT3(0, 0, -1), true);
					}
				//}
			}
		}
	}
	//ReserveUpdate(partitionPos, true);
	//ReserveUpdate(partitionPos + XMFLOAT3(-1, 0, 0), true);
	//ReserveUpdate(partitionPos + XMFLOAT3(-1, -1, 0), true);
	//ReserveUpdate(partitionPos + XMFLOAT3(-1, -1, -1), true);
	//ReserveUpdate(partitionPos + XMFLOAT3(0, -1, 0), true);
	//ReserveUpdate(partitionPos + XMFLOAT3(0, -1, -1), true);
	//ReserveUpdate(partitionPos + XMFLOAT3(-1, 0, -1), true);
	//ReserveUpdate(partitionPos + XMFLOAT3(0, 0, -1), true);
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


VoxelComp::MESH_RESULT VoxelComp::UpdatePartialMesh(XMFLOAT3 pos, int lodLevel, short transitionCellBasis)
{
	MESH_RESULT buffer;
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

void VoxelComp::UpdateVoxelMesh()
{
	for(int i=0;i<width;i+=partitionSize)
		for (int j = 0; j<height; j += partitionSize)
			for (int k = 0; k<depth; k += partitionSize)
				GenerateCubeFaces(XMFLOAT3(i,j,k));
}

void VoxelComp::UpdateMarchingCubeMesh(int lodLevel)
{
	ProcessUpdateQueue();
	/*if (useMarchingCube)
	{
		for (int i = 0; i<width; i += partitionSize)
			for (int j = 0; j<height; j += partitionSize)
				for (int k = 0; k < depth; k += partitionSize)
				{
					MESH_RESULT buffer = GeneratePartitionFaces(XMFLOAT3(i, j, k), lodLevel);
					UpdateMeshRenderer(buffer.newMesh, buffer.pos,buffer.lodLevel);
				}
	}*/
}
XMFLOAT3 VoxelComp::GetPositionFromIndex(int index)
{
	int x = (int)index % width;
	int y = ((int)index / width) % height;
	int z = (int)index / (width* height);
	return XMFLOAT3(x, y, z);
}
int VoxelComp::GetIndexFromPosition(XMFLOAT3 pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.z < 0)
		return -1;
	if (pos.x >= width || pos.y >= height || pos.z >= depth)
		return -1;
	return (int)pos.x + (int)pos.y*width + (int)pos.z*width*height;
}
void VoxelComp::RefreshLODNodes()
{

	XMFLOAT3 basePos = GetPartitionStartPos(lastBasePosition);
	std::unordered_map<int, LODGroupData> newLODGroupsLoaded;
	std::unordered_map<int, LODGroupData> newLODGroups;
	std::unordered_map<int, LODGroupData> oldLODGroups;
	oldLODGroups.swap(lodGropups);
	int lodDistance = LODDistance[maxLODLevel - 1];
	int min = -lodDistance - partitionSize;
	int max = lodDistance + partitionSize;

	for (int i = min; i < max; i += partitionSize)
	{
		for (int j = min; j < max; j += partitionSize)
		{
			for (int k = min; k < max; k += partitionSize)
			{
				XMFLOAT3 targetPos = basePos + XMFLOAT3(i, j, k);
				if (targetPos.x < 0 || targetPos.y < 0 || targetPos.z < 0 || targetPos.x >= width || targetPos.y >= height || targetPos.z >= depth)
					continue;

				LODGroupData lodGroupData;
				lodGroupData.level = GetLODLevel(basePos, targetPos);

				lodGroupData.transitionBasis = GetTransitionBasis(lodGroupData.level, basePos, targetPos);
				int index = (int)targetPos.x + (int)targetPos.y*width + (int)targetPos.z*width*height;
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
		ReserveUpdate(GetPositionFromIndex(i.first), i.second.transitionBasis, i.second.level, false, false);
		lodGropups[i.first] = i.second;
	}
	for (auto i : oldLODGroups)
	{
		ReserveUpdate(GetPositionFromIndex(i.first), false, false);
	}
	oldLODGroups.clear();


	//for (auto i : lodGropups)
	//{
	//	ReserveUpdate(GetPositionFromIndex(i.first), false, true);
	//}
	//lodGropups.clear();
	//std::unordered_map<int, LODGroupData> newLODGroups;
	//XMFLOAT3 basePos = GetPartitionStartPos(lastBasePosition);
	//int lodDistance = LODDistance[maxLODLevel - 1];
	//int min = -lodDistance - partitionSize;
	//int max = lodDistance + partitionSize;
	//for (int i = min; i < max; i += partitionSize)
	//	for (int j = min; j < max; j += partitionSize)
	//		for (int k = min; k < max; k += partitionSize)
	//		{
	//			XMFLOAT3 targetPos = XMFLOAT3(i, j, k) + basePos;
	//			int level = GetLODLevel(basePos, targetPos);//ReserveUpdate(targetPos, false, false);
	//			if (level >= 0)
	//			{
	//				int index = GetIndexFromPosition(targetPos);
	//				LODGroupData data;
	//				data.level = level;
	//				data.transitionBasis = GetTransitionBasis(level, basePos, targetPos);
	//				auto iter = lodGropups.find(index);
	//				if (iter != lodGropups.end())
	//				{
	//					if (iter->second.level == data.level&&iter->second.transitionBasis == data.transitionBasis)
	//					{

	//					}
	//				}
	//				newLODGroups[GetIndexFromPosition(targetPos)] = data;
	//			}
	//		}
	//newLODGroups.swap(lodGropups);
}
void VoxelComp::RefreshLODNodes(XMFLOAT3 basePos)
{


	basePos = GetPartitionStartPos(basePos);
	std::unordered_map<int, LODGroupData> newLODGroupsLoaded;
	std::unordered_map<int, LODGroupData> newLODGroups;
	std::unordered_map<int, LODGroupData> oldLODGroups;
	oldLODGroups.swap(lodGropups);
	int lodDistance = LODDistance[maxLODLevel - 1];
	int min = -lodDistance-partitionSize;
	int max = lodDistance+partitionSize;

	for (int i = min; i < max; i+=partitionSize)
	{
		for (int j= min; j < max; j+=partitionSize)
		{
			for (int k = min; k < max; k+=partitionSize)
			{
				XMFLOAT3 targetPos = basePos + XMFLOAT3(i, j, k);
				if (targetPos.x < 0 || targetPos.y < 0 || targetPos.z < 0 || targetPos.x >= width || targetPos.y >= height || targetPos.z >= depth)
					continue;

				LODGroupData lodGroupData;
				lodGroupData.level = GetLODLevel(basePos, targetPos);

				lodGroupData.transitionBasis = GetTransitionBasis(lodGroupData.level, basePos, targetPos);
				int index = (int)targetPos.x + (int)targetPos.y*width + (int)targetPos.z*width*height;
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
		ReserveUpdate(GetPositionFromIndex(i.first),i.second.transitionBasis,i.second.level, false, false);
		lodGropups[i.first] = i.second;
	}
	for (auto i : oldLODGroups)
	{
		ReserveUpdate(GetPositionFromIndex(i.first), false, false);
	}
	oldLODGroups.clear();

}

void VoxelComp::ProcessLOD()
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
				ReserveUpdate(XMFLOAT3(i,j,k), false,false);
			}
		}
	}
}

void VoxelComp::LoadHeightMapFromRaw(int _width, int _height,int _depth, int _maxHeight, const char* filename,int startX, int startZ, int endX, int endZ)
{
	NewChunks(_width, _height, _depth);
	unsigned short** data = nullptr;
	int top=0,bottom=0;
	ReadRawEX16(data, filename, width, depth,top,bottom);
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
				if ((x%partitionSize == 0) && (y%partitionSize == 0) && (z%partitionSize == 0))
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
					ReserveUpdate(XMFLOAT3(x, y, z), false, false);
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
				if (y <= noise)
				{
					vox.material = 1;
				}
				else
				{
				}
				vox.isoValue = noise - y;
				SetVoxel(x, y, z, vox,true);
				if ((x%partitionSize == 0) && (y%partitionSize == 0) && (z%partitionSize == 0))
					ReserveUpdate(XMFLOAT3(x, y, z), false,false);
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
	if (useArrayedOctree)
	{
		aOctree = new ArrayedOctree<VoxelData>(_w);
	}
	else if (!useSpartialMatrix)
	{
		chunks = new VoxelData[_w*_h*_d];
	}
	NewOctree(_w);
}
void VoxelComp::NewOctree(int _size)
{
	//if (_size % 2 > 0)
	//	_size--;
	gOctreeMeshRenderer = new Octree<MeshRenderer*>(_size,partitionSize);
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

XMFLOAT3 VoxelComp::GetPartitionCenter(XMFLOAT3 basePos)
{
	float hSize = partitionSize * 0.5f;
	return GetPartitionStartPos(basePos) + XMFLOAT3(hSize, hSize, hSize);
}
XMFLOAT3 VoxelComp::GetPartitionStartPos(XMFLOAT3 basePos)
{
	float x, y, z;
	float size = 1 / (float)partitionSize;
	x = floor((basePos.x *size));
	y = floor((basePos.y *size));
	z = floor((basePos.z *size));
	return XMFLOAT3(x, y, z)*partitionSize;
}

int VoxelComp::GetLODLevel(const XMFLOAT3& basePos, const XMFLOAT3& targetPos)
{
	if (targetPos.x < 0 || targetPos.y < 0 || targetPos.z < 0 || targetPos.x >= width || targetPos.y >= height || targetPos.z >= depth)
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

void VoxelComp::SetLODLevel(int level, float distance)
{
	if (level < 0 || level >= 8)
		return;
	distance -= (int)distance % partitionSize;
	LODDistance[level] = distance;
}

void VoxelComp::ProcessUpdateQueue()
{
	if (!useAsyncBuild)
	{
		if (updateQueue_Main.size())
		{
			//ULONG tick = GetTickCount64();
			for (auto i : updateQueue_Main)
			{
				MESH_RESULT result = UpdatePartialMesh(XMFLOAT3(i.x, i.y, i.z), i.lodLevel, i.transitionCellBasis);
				UpdateMeshRenderer(result.newMesh, result.pos, result.lodLevel);
			}
			updateQueue_Main.clear();
			//printf("updateQueue_Main Generated %dms\n", GetTickCount64() - tick);
		}
		if (updateQueue_Deform.size())
		{
			//ULONG tick = GetTickCount64();
			for (auto i : updateQueue_Deform)
			{
				MESH_RESULT result = UpdatePartialMesh(XMFLOAT3(i.x, i.y, i.z), i.lodLevel, i.transitionCellBasis);
				UpdateMeshRenderer(result.newMesh, result.pos, result.lodLevel);
			}
			updateQueue_Deform.clear();
			//printf("updateQueue_Deform Generated %dms\n", GetTickCount64() - tick);
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
			//ULONG tick = GetTickCount64();
			for (auto i : *(threadPool_Main.GetResultQueue()))
			{
				UpdateMeshRenderer(i.newMesh, i.pos,i.lodLevel);
			}
			threadPool_Main.GetResultQueue()->clear();
			//printf("threadPool_Main Updated %dms\n", GetTickCount64() - tick);
		}
	}
	if (threadPool_Deform.isInit)
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
short VoxelComp::GetTransitionBasis(int lodLevel, XMFLOAT3 basePos, XMFLOAT3 targetPos)
{
	if (lodLevel == 0)
		return 0;
	short transitionCellBasis = 0;
	XMFLOAT3 r(partitionSize, 0, 0), u(0, partitionSize, 0), f(0, 0, partitionSize);
	int l[6] = { GetLODLevel(basePos, targetPos - f),GetLODLevel(basePos, targetPos + f),GetLODLevel(basePos, targetPos - r),GetLODLevel(basePos, targetPos + r),GetLODLevel(basePos, targetPos - u),GetLODLevel(basePos, targetPos + u) };
	for (int i = 0; i < 6; i++)
		if (l[i] >= 0 && l[i] < lodLevel)
			transitionCellBasis |= (1 << i);
	return transitionCellBasis;
}
short VoxelComp::ReserveUpdate(XMFLOAT3 pos, short _basis, short _lodLevel, bool isDeforming, bool checkDuplicated)
{
	if (pos.x >= width || pos.y >= height || pos.z >= depth)
		return -1;
	if (pos.x < 0 || pos.y < 0 || pos.z < 0)
		return -1;
	INPUT_BUFFER input;
	XMFLOAT3 targetPos = GetPartitionStartPos(pos);
	input.x = (int)targetPos.x;
	input.y = (int)targetPos.y;
	input.z = (int)targetPos.z;
	input.lodLevel = _lodLevel;
	input.transitionCellBasis = _basis;
	if (checkDuplicated)
	{
		if (isDeforming)
		{
			for (auto i : updateQueue_Deform)
				if (i == input)
					return -1;
		}
		else
		{
			for (auto i : updateQueue_Main)
				if (i == input)
					return -1;
		}
	}
	if (!isDeforming)
		updateQueue_Main.push_back(input);
	else
		updateQueue_Deform.push_back(input);
	return input.lodLevel;
}

short VoxelComp::ReserveUpdate(XMFLOAT3 pos, bool isDeforming,bool checkDuplicated)
{
	//mutex_UpdateQueue.lock();
	if (pos.x >= width || pos.y >= height || pos.z >= depth)
		return -1;
	if (pos.x < 0 || pos.y < 0 || pos.z < 0)
		return  - 1;
	INPUT_BUFFER input;
	XMFLOAT3 targetPos = GetPartitionStartPos(pos);
	input.x = (int)targetPos.x;
	input.y = (int)targetPos.y;
	input.z = (int)targetPos.z;
	XMFLOAT3 basePos = GetLastBasePosition();
	basePos = GetPartitionStartPos(basePos);
	input.lodLevel = GetLODLevel(basePos, targetPos);
	input.transitionCellBasis = GetTransitionBasis(input.lodLevel,basePos, targetPos);
	if (checkDuplicated)
	{
		if (isDeforming)
		{
			for (auto i : updateQueue_Deform)
				if (i == input)
					return  - 1;
		}
		else
		{
			for (auto i : updateQueue_Main)
				if (i == input)
					return  - 1;
		}
	}
	if (!isDeforming)
		updateQueue_Main.push_back(input);
	else
		updateQueue_Deform.push_back(input);
	//mutex_UpdateQueue.unlock();
	return input.lodLevel;
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