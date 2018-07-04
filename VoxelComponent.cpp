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
	ReleaseChunks();
	ReleaseOctree();
	if (material)
	{
		delete material;
	}
	material = 0;
	if (T_BuildThread)
	{
		CloseHandle(T_BuildThread);
	}
	T_BuildThread = 0;
}
void VoxelComponent::SetMeshToRenderer(Mesh* newMesh, OctreeNode<MeshRenderer*>* rendererNode)
{
	if (newMesh)
	{
		if (rendererNode->GetValue() == NULL)
		{
			GameObject* gobj = new GameObject("voxel partition");
			MeshRenderer* newRenderer = new MeshRenderer;
			newRenderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_texture"));
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
void VoxelComponent::Update()
{
	if (vertBuildState == VERT_BUILD_FINISHED)
	{
		for (auto i : asyncBuildResult)
		{
			SetMeshToRenderer(i.newMesh, i.rendererNode);
		}
		asyncBuildResult.clear();
		vertBuildState = VERT_BUILD_NONE;
	}
	CheckLOD();
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
	if (Input()->GetKey(DIK_PGUP))
	{
		currentOctreeDepth++;
		if (currentOctreeDepth > aOctree->depth)
			currentOctreeDepth = aOctree->depth;
		UpdateMesh(true);
	}
	else if (Input()->GetKey(DIK_PGDN))
	{
		currentOctreeDepth--;
		if (currentOctreeDepth < 1)
			currentOctreeDepth = 1;
		UpdateMesh(true);
	}
	if (vertBuildState == VERT_BUILD_NONE)
	{
		if (Input()->GetKey(VK_RBUTTON))
		{
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
							if(SetChunk((int)x, (int)y, (int)z, vox))
								uSet.insert(gOctree->GetPartialNode(XMFLOAT3(x, y, z), partitionSize));
							//isUpdated = true;
						}
					}
				}
			}
			if (isUpdated)
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
							if(SetChunk((int)x, (int)y, (int)z, vox))
								uSet.insert(gOctree->GetPartialNode(XMFLOAT3(x, y, z), partitionSize));
							//isUpdated = true;
						}
					}
				}
			}
			if (isUpdated)
				UpdateMesh(false);
		}
		if (uSet.size() && vertBuildState == VERT_BUILD_NONE)
		{
			for (auto i : uSet)
				UpdatePartialMesh(i->GetPosition());
			uSet.clear();
			//if (!useMultiThread)
			//{
			//	for (auto i : uSet)
			//		UpdatePartialMesh(i->GetPosition());
			//	uSet.clear();
			//}
			//else
			//{
			//	printf("%d �� ������Ʈ\n", uSet.size());
			//	UpdatePartialMeshAsync(this, 0);
			//}
		}
	}
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


	unit = 1.0f;
	tUnit = 0.25f;
	tAmount = 4;

	useMarchingCube = true;
	useOctree = true;
	useGPGPU = false;
	useGeometry = false;
	useMultiThread = false;
	//useFrustum = true;

	partitionSize = 16;

	SetLODLevel(0, 150);
	SetLODLevel(1, 200);
	SetLODLevel(2, 300);
	SetLODLevel(3, 500);
	lastBasePosition = CameraComponent::mainCamera()->transform()->GetWorldPosition();
	

	if (useGeometry&&useMarchingCube)
	{
		material = new Material(*ResourcesClass::GetInstance()->FindMaterial("m_marchingCube"));
		renderer->SetMaterial(material);
	}
	else
		renderer->SetMaterial(ResourcesClass::GetInstance()->FindMaterial("m_texture"));


	//LoadCube(32, 32, 32);
	LoadPerlin(32, 32, 32, 64, 0.3);
	//int h = ReadTXT("/data/height.txt");




	ULONG tick = GetTickCount64();
	//LoadHeightMapFromRaw(1025, 512, 1025, "data/heightmap.r16");// , 0, 0, 255, 255);
	if (useOctree && !gOctree)
		BuildOctree(width, lastBasePosition);
	if (!useMultiThread)
	{
		if (useOctree)
			currentOctreeDepth = 0;
		UpdateMesh(true);
	}
	else UpdateMeshAsync(this, 0);
	////AOctree
	//if(useOctree&&!aOctree)
	//	BuildOctree(width, lastBasePosition);
	//if (!useMultiThread)
	//{
	//	if(useOctree)
	//		currentOctreeDepth = aOctree->depth;
	//	UpdateMesh(true);
	//}
	//else BuildVertexBufferAsync(this, aOctree->depth);
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
	if (gOctree)
		delete gOctree;
	gOctree = 0;
}
void VoxelComponent::CreateFaceUp(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComponent::CreateFaceDown(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComponent::CreateFaceRight(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComponent::CreateFaceLeft(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComponent::CreateFaceForward(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComponent::CreateFaceBackward(float x, float y, float z, float _unit, BYTE type, int& faceCount, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
void VoxelComponent::CreateFaceMarchingCube(float x, float y, float z, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
		VoxelData pData = gOctree->GetNodeAtPosition(p3,0)->GetValue();
		value0 = pData.isoValue;
		value1 = gOctree->GetNodeAtPosition(px3, 0)->GetValue().isoValue;
		value2 = gOctree->GetNodeAtPosition(py3, 0)->GetValue().isoValue;
		value3 = gOctree->GetNodeAtPosition(pxy3, 0)->GetValue().isoValue;
		value4 = gOctree->GetNodeAtPosition(pz3, 0)->GetValue().isoValue;
		value5 = gOctree->GetNodeAtPosition(pxz3, 0)->GetValue().isoValue;
		value6 = gOctree->GetNodeAtPosition(pyz3, 0)->GetValue().isoValue;
		value7 = gOctree->GetNodeAtPosition(pxyz3, 0)->GetValue().isoValue;
		////AOctree
		//VoxelData pData = aOctree->GetValue(p);
		//value0 = pData.isoValue;
		//value1 = aOctree->GetValue(px).isoValue;
		//value2 = aOctree->GetValue(py).isoValue;
		//value3 = aOctree->GetValue(pxy).isoValue;
		//value4 = aOctree->GetValue(pz).isoValue;
		//value5 = aOctree->GetValue(pxz).isoValue;
		//value6 = aOctree->GetValue(pyz).isoValue;
		//value7 = aOctree->GetValue(pxyz).isoValue;
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
void VoxelComponent::BuildVertexBufferGS(int targetDepth, std::vector<VertexBuffer>& vertices)
{
	if (useFrustum)
	{
		BuildVertexBufferFrustumCulling(targetDepth);
		return;
	}
	int length = width;
	vertices.clear();
	float _unit = unit;
	if (useOctree)
	{
		_unit = aOctree->GetUnitSize(targetDepth);
		length = aOctree->GetLength(targetDepth);
	}
	ULONG time2 = GetTickCount();
	vertices.resize(length*length*length);
	for (int i = 0; i < length - 1; i++)
		for (int j = 0; j < length - 1; j++)
			for (int k = 0; k < length - 1; k++)
			{
				XMFLOAT3 pos = XMFLOAT3(i*_unit, j*_unit, k*_unit);
				VertexBuffer vert;
				vert.position = XMFLOAT3(pos);
				vertices[i + j * length + k * length*length] = vert;
				//vertices.push_back(vert);
			}
	unit = _unit;
	Material* material = renderer->GetMaterial();
	material->GetParams()->SetInt("vertCount", vertices.size());
	material->GetParams()->SetInt("length", length);
	material->GetParams()->SetFloat2("unitSize", XMFLOAT2(unit, 0));
	printf("Marching Cube Build Data ( Geometry, useOctree=%d ): %d ms\n", useOctree, GetTickCount() - time2);
}

//��Īť�� ������ ����

void VoxelComponent::GenerateMarchingCubeFaces(std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices,bool isNew)
{
	ULONG time = GetTickCount();
	vertices.clear();
	indices.clear();
	for (int x = 0; x < width-1; x++)
	{
		for (int y = 0; y < height-1; y++)
		{
			for (int z = 0; z < depth-1; z++)
			{
				CreateFaceMarchingCube(x, y, z, unit,vertices,indices);
			}
		}
	}
	printf("Marching Cube Create Buffer ( CPU ): %d ms\n", GetTickCount() - time);
	
}
void VoxelComponent::GenerateMarchingCubeFaces_GS(std::vector<VertexBuffer>& vertices, bool isNew)
{
	ULONG time = GetTickCount();
	int length = width;
	if (isNew&&!useMultiThread)
	{
		BuildVertexBufferGS(currentOctreeDepth,vertices);
	}
	Material* material = renderer->GetMaterial();
	ULONG time3 = GetTickCount();
	//SetupMarchingCubeVertexBufferGS();
	//printf("Marching Cube Build Vertex Buffer ( Geometry ): %d ms\n", GetTickCount() - time3);
	if (vertices.size() > 0)
	{
		material->GetParams()->SetFloat("isoLevel", isoLevel);
		ID3D11Device* device = SystemClass::GetInstance()->GetDevice();

		if (useOctree)
		{
			length = aOctree->GetLength(currentOctreeDepth);
		}
		int nums = nums = length * length*length;
		if (useOctree)
		{
			material->GetParams()->SetSRV("chunksData", new StructuredBuffer(device, sizeof(VoxelData), nums, aOctree->GetNodes(NULL, currentOctreeDepth)));
		}
		else material->GetParams()->SetSRV("chunksData", new StructuredBuffer(device, sizeof(VoxelData), nums, chunksData));
	}
	printf("Marching Cube Create Buffer ( Geometry ): %d ms\n", GetTickCount() - time);
}
void VoxelComponent::GenerateMarchingCubeFaces_GPGPU(std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, bool isNew)
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
			//	CreateFaceMarchingCube(x, y, z, unit);
			}
		}
	}
	printf("Marching Cube Create Buffer ( GPU ): %d ms\n", GetTickCount() - time);
}
void VoxelComponent::GenerateMarchingCubeFaces_Octree(XMFLOAT3 pos, bool isAsync)
{
	ULONG time = GetTickCount();
	std::vector<OctreeNode<VoxelData>*> leafs;

	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	OctreeNode<VoxelData>* node = gOctree->GetPartialNode(pos, partitionSize);
	node->GetLeafs(leafs);
	for (auto i : leafs)
	{
		int x = i->GetCenerPosition().x;
		int y = i->GetCenerPosition().y;
		int z = i->GetCenerPosition().z;
		CreateFaceMarchingCube(x, y, z, unit, vertices, indices);
	}
	printf("Marching Cube Create Buffer ( CPU ): %d ms\n", GetTickCount() - time);
	CreatePartialMesh(pos, node, vertices, indices,isAsync);
}


void VoxelComponent::GenerateVoxelFaces(std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices)
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
					CreateFaceLeft(x*unit, y*unit, z*unit,unit, myBlock, faceCount,vertices,indices);
				}
				if (GetChunk(x + 1, y, z).material == 0)//RIGHT
				{
					CreateFaceRight(x*unit, y*unit, z*unit,unit, myBlock, faceCount,vertices,indices);
				}
				if (GetChunk(x, y + 1, z).material == 0)//UP
				{
					CreateFaceUp(x*unit, y*unit, z*unit,unit, myBlock, faceCount,vertices,indices);
				}
				if (GetChunk(x, y - 1, z).material == 0)//DOWN
				{
					CreateFaceDown(x*unit, y*unit, z*unit,unit, myBlock, faceCount,vertices,indices);
				}
				if (GetChunk(x, y, z + 1).material == 0)//FORWARD
				{
					CreateFaceForward(x*unit, y*unit, z*unit,unit, myBlock, faceCount,vertices,indices);
				}
				if (GetChunk(x, y, z - 1).material == 0)//BACKWARD
				{
					CreateFaceBackward(x*unit, y*unit, z*unit,unit, myBlock, faceCount,vertices,indices);
				}
			}
		}
	}
}

void VoxelComponent::GenerateOctreeFaces(XMFLOAT3 pos)
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
			CreateFaceLeft(x*unit, y*unit, z*unit, unit, myBlock, faceCount,vertices,indices);
		}
		if (GetChunk(x + 1, y, z, node).material == 0)//RIGHT
		{
			CreateFaceRight(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
		if (GetChunk(x, y + 1, z, node).material == 0)//UP
		{
			CreateFaceUp(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
		if (GetChunk(x, y - 1, z, node).material == 0)//DOWN
		{
			CreateFaceDown(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
		if (GetChunk(x, y, z + 1, node).material == 0)//FORWARD
		{
			CreateFaceForward(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
		if (GetChunk(x, y, z - 1, node).material == 0)//BACKWARD
		{
			CreateFaceBackward(x*unit, y*unit, z*unit, unit, myBlock, faceCount, vertices, indices);
		}
	}

	CreatePartialMesh(pos, node, vertices, indices);
	/*
	//AOctree
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
	*/
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

VoxelData VoxelComponent::GetChunk(int x, int y, int z, OctreeNode<VoxelData>* _startNode)
{
	VoxelData v;
	v.material = 0;
	if (x >= width || y >= height || z >= depth)
		return v;
	if (x < 0 || y < 0 || z < 0)
		return v;
	if (useOctree&&!chunksData) 
	{
		if (_startNode)
		{
			if ((x < _startNode->GetPosition().x || x >= _startNode->GetPosition().x + _startNode->GetCellSize())
				|| (y < _startNode->GetPosition().y || y >= _startNode->GetPosition().y + _startNode->GetCellSize())
				|| (z < _startNode->GetPosition().z || z >= _startNode->GetPosition().z + _startNode->GetCellSize()))
				_startNode = NULL;
		}
		VoxelData v=gOctree->GetNodeAtPosition(XMFLOAT3(x, y, z), 0, _startNode)->GetValue();
		return v;
		////AOctree
		//return aOctree->GetValue(XMFLOAT3(x, y, z));
	}
	return chunksData[x + y * width + z * width*height];
}



OctreeNode<VoxelData>* VoxelComponent::SetChunk(int x, int y, int z, VoxelData value)
{
	if (x >= width || y >= height || z >= depth)
		return NULL;
	if (x < 0 || y < 0 || z < 0)
		return NULL;
	if (useOctree && gOctree)
	{
		XMFLOAT3 pos(x, y, z);
		VoxelData vox = value;
		VoxelData lastVox;
		lastVox = vox;
		OctreeNode<VoxelData>* node = gOctree->root->Subdivide(pos,0,true);
		node->SetValue(vox);
		return node;
	}
	////AOctree
	//if (useOctree && aOctree)
	//{
	//	XMFLOAT3 pos(x, y, z);
	//	VoxelData vox = value;
	//	VoxelData lastVox;
	//	lastVox = vox;
	//	int idx = aOctree->GetNodeIDX(pos);
	//	aOctree->SetValue(idx, vox, aOctree->depth);
	//	for (int i = aOctree->depth-1; i >= 0; i--)
	//	{
	//		int idx = aOctree->GetNodeIDX(pos, i);
	//		int idxChild = aOctree->GetNodeIDX(pos, i+1);
	//		lastVox = aOctree->GetValue(idxChild, i + 1);
	//		aOctree->SetValue(idx, aOctree->GetValue(idxChild,i+1), i);
	//	}
	//}
	else
	{
		if (value.isoValue < isoLevel)
			value.material = 0;
		else value.material = 1;
		chunksData[x + y * width + z * width*height] = value;
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

void VoxelComponent::UpdateMesh(bool isNew)
{
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

void VoxelComponent::CreatePartialMesh(XMFLOAT3 pos, OctreeNode<VoxelData>* node,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices,bool isAsync)
{
	OctreeNode<MeshRenderer*>* rendererNode;
	rendererNode = gOctreeMeshRenderer->root->Subdivide(pos, node->GetDepth(), true);
	Mesh* newMesh = new Mesh();
	if (vertices.size())
	{
		newMesh = new Mesh();
		newMesh->SetVertices(&vertices[0], vertices.size());
		if (indices.size())
			newMesh->SetIndices(&indices[0], indices.size());
		if (!useGeometry)
			newMesh->RecalculateNormals();
		newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
		if (isAsync)
		{
			ASYNC_BUILD_BUFFER buffer;
			buffer.newMesh = newMesh;
			buffer.rendererNode = rendererNode;
			asyncBuildResult.push_back(buffer);
		}
		else
			SetMeshToRenderer(newMesh, rendererNode);
	}
	else
	{
		if (rendererNode->GetValue())
		{
			if (isAsync)
			{
				ASYNC_BUILD_BUFFER buffer;
				buffer.newMesh = NULL;
				buffer.rendererNode = rendererNode;
				asyncBuildResult.push_back(buffer);
			}
			else
				SetMeshToRenderer(newMesh, rendererNode);
		}
	}

}


void VoxelComponent::UpdatePartialMesh(XMFLOAT3 pos, bool isAsync)
{
	if (useOctree)
	{
		ULONG tick = GetTickCount64();
		if (!useMarchingCube)
		{
			//pos = gOctree->GetNodeAtPosition(pos, 0)->GetPosition();
			GenerateOctreeFaces(pos);
		}
		else
		{
			GenerateMarchingCubeFaces_Octree(pos, isAsync);
		}
		printf("Update Mesh : %dms\n", GetTickCount64() - tick);
	}
}

void VoxelComponent::UpdateVoxelMesh()
{
	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	if (!useOctree)
		GenerateVoxelFaces(vertices,indices);
	else
	{
		for(int i=0;i<width;i+=partitionSize)
			for (int j = 0; j<height; j += partitionSize)
				for (int k = 0; k<depth; k += partitionSize)
					GenerateOctreeFaces(XMFLOAT3(i,j,k));
		return;
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
	Mesh* lastMesh = renderer->GetMesh();
	renderer->SetMesh(newMesh);
	if (lastMesh)
	{
		lastMesh->ShutdownBuffers();
		delete lastMesh;
	}
}

void VoxelComponent::UpdateMarchingCubeMesh(bool isNew)
{
	std::vector<VertexBuffer> vertices;
	std::vector<unsigned long> indices;
	if (useMarchingCube)
	{
		if (useGPGPU)
			GenerateMarchingCubeFaces_GPGPU(vertices,indices,isNew);
		else if (useOctree && !useGeometry)
		{
			for (int i = 0; i<width; i += partitionSize)
				for (int j = 0; j<height; j += partitionSize)
					for (int k = 0; k<depth; k += partitionSize)
						GenerateMarchingCubeFaces_Octree(XMFLOAT3(i, j, k));
			return;
		}
		else if(useGeometry)
			GenerateMarchingCubeFaces_GS(vertices,isNew);
		else
			GenerateMarchingCubeFaces(vertices, indices, isNew);
	}
	else
		return;
	if (!useGeometry || isNew )
	{
		if (useMultiThread&&vertBuildState!= VERT_BUILD_FINISHED)
		{
			return;
		}

		if (vertices.size() > 0)
		{
			Mesh* newMesh = new Mesh();
			if (vertices.size())
				newMesh->SetVertices(&vertices[0], vertices.size());
			if (indices.size() && !useGeometry)
				newMesh->SetIndices(&indices[0], indices.size());
			/*if (!useGeometry)
				newMesh->RecalculateNormals();*/
			newMesh->InitializeBuffers(SystemClass::GetInstance()->GetDevice());
			Mesh* lastMesh = renderer->GetMesh();
			renderer->SetMesh(newMesh);
			if (lastMesh)
			{
				lastMesh->ShutdownBuffers();
				delete lastMesh;
			}
		}
		else
		{
			Mesh* lastMesh = renderer->GetMesh();
			if (lastMesh)
			{
				lastMesh->ShutdownBuffers();
				delete lastMesh;
			}
			renderer->SetMesh(NULL);
		}
	}
}

void VoxelComponent::SetVertexBuildState(VERT_BUILD_STATE _state)
{
	vertBuildState = _state;
}

void VoxelComponent::CheckLOD()
{
	if (!useOctree)
		return;
	//if (useMultiThread)
	//{
	//	if (vertBuildState == VERT_BUILD_NONE)
	//	{
	//		XMFLOAT3 camPos = camera->transform->GetWorldPosition();
	//		XMFLOAT3 centerPos = transform()->GetWorldPosition() + XMFLOAT3(width*0.5f, height*0.5f, depth*0.5f);
	//		int lodLevel = GetLODLevel(camPos, centerPos);
	//		if (currentOctreeDepth != aOctree->depth - lodLevel)
	//		{
	//			BuildVertexBufferAsync(this, aOctree->depth - lodLevel);
	//		}
	//	}
	//	else if (vertBuildState == VERT_BUILD_FINISHED)
	//	{
	//		UpdateMesh(true);
	//		CloseHandle(T_BuildThread);
	//		vertBuildState = VERT_BUILD_NONE;
	//	}
	//}
	//else
	//{
	//	XMFLOAT3 camPos = camera->transform->GetWorldPosition();
	//	XMFLOAT3 centerPos = transform()->GetWorldPosition() + XMFLOAT3(width*0.5f, height*0.5f, depth*0.5f);
	//	int lodLevel = GetLODLevel(camPos, centerPos);
	//	if (currentOctreeDepth != aOctree->depth - lodLevel)
	//	{
	//		currentOctreeDepth = aOctree->depth-lodLevel;
	//		UpdateMesh(true);
	//	}
	//}
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
		// ���� �ݰ��� �� frustum �ȿ� �ִ��� Ȯ���մϴ�.
		if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], XMVectorSet(xCenter, yCenter, zCenter, 1.0f))) < -radius)
			return false;
	}

	return true;
}


void VoxelComponent::ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix)
{
	XMFLOAT4X4 pMatrix;
	XMStoreFloat4x4(&pMatrix, projectionMatrix);

	// ����ü���� �ּ� Z �Ÿ��� ����մϴ�.
	float zMinimum = -pMatrix._43 / pMatrix._33;
	float r = screenDepth / (screenDepth - zMinimum);

	// ������Ʈ �� ���� �ٽ� ���� ��Ŀ� �����մϴ�.
	pMatrix._33 = r;
	pMatrix._43 = -r * zMinimum;
	projectionMatrix = XMLoadFloat4x4(&pMatrix);

	// �� ��Ʈ������ ������Ʈ �� �������� ��Ʈ�������� ����ü ��Ʈ������ ����ϴ�.
	XMMATRIX finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

	// ���� ����� XMFLOAT4X4 �������� ��ȯ�մϴ�.
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, finalMatrix);

	// ����ü�� ����� ����� ����մϴ�.
	float x = (float)(matrix._14 + matrix._13);
	float y = (float)(matrix._24 + matrix._23);
	float z = (float)(matrix._34 + matrix._33);
	float w = (float)(matrix._44 + matrix._43);
	m_planes[0] = XMVectorSet(x, y, z, w);
	m_planes[0] = XMPlaneNormalize(m_planes[0]);

	// ����ü�� �� ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[1] = XMVectorSet(x, y, z, w);
	m_planes[1] = XMPlaneNormalize(m_planes[1]);

	// ����ü�� ���� ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[2] = XMVectorSet(x, y, z, w);
	m_planes[2] = XMPlaneNormalize(m_planes[2]);

	// ����ü�� ������ ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[3] = XMVectorSet(x, y, z, w);
	m_planes[3] = XMPlaneNormalize(m_planes[3]);

	// ����ü�� �� ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._13);
	y = (float)(matrix._24 + matrix._23);
	z = (float)(matrix._34 + matrix._33);
	w = (float)(matrix._44 + matrix._43);
	m_planes[4] = XMVectorSet(x, y, z, w);
	m_planes[4] = XMPlaneNormalize(m_planes[4]);

	// ����ü�� �Ʒ� ����� ����մϴ�.
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
	int length = width;
	std::vector<VertexBuffer> vertices;
	float _unit = unit;
	if (useOctree)
	{
		_unit = aOctree->GetUnitSize(targetDepth);
		length = aOctree->GetLength(targetDepth);
	}
	CameraComponent* camComp=camera->GetComponent<CameraComponent>();
	XMMATRIX proj, view;
	camComp->GetProjectionMatrix(proj);
	camComp->Render();
	camComp->GetViewMatrix(view);
	ConstructFrustum(camComp->m_farPlane, proj, view);
	ULONG time2 = GetTickCount();
	vertices.reserve(length*length*length);
	for (int i = 0; i < length - 1; i++)
		for (int j = 0; j < length - 1; j++)
			for (int k = 0; k < length - 1; k++)
			{
				XMFLOAT3 pos = XMFLOAT3(i*_unit, j*_unit, k*_unit);
				float halfUnit = _unit * 0.5f;
				XMFLOAT3 frustumPos = pos + transform()->GetWorldPosition()+XMFLOAT3(halfUnit, halfUnit, halfUnit);
				//if (FrustumCheckSphere(frustumPos.x, frustumPos.y, frustumPos.z, halfUnit))
				if (FrustumCheckCube(frustumPos.x , frustumPos.y , frustumPos.z, halfUnit))
				{
					VertexBuffer vert;
					vert.position = XMFLOAT3(pos);
					vertices.push_back(vert);
				}
			}
	unit = _unit;
	Material* material = renderer->GetMaterial();
	material->GetParams()->SetInt("vertCount", vertices.size());
	material->GetParams()->SetInt("length", length);
	material->GetParams()->SetFloat2("unitSize", XMFLOAT2(unit, 0));
	printf("Marching Cube Build Data ( Geometry, useOctree=%d ): %d ms\n", useOctree, GetTickCount() - time2);
}

void VoxelComponent::UpdateMeshAsync(VoxelComponent * voxel, int targetDepth)
{
	voxel->SetVertexBuildState(VERT_BUILD_LOADING);
	VertAsyncBuffer* buffer=new VertAsyncBuffer;
	buffer->voxel = voxel;
	buffer->targetDepth = targetDepth;
	voxel->T_BuildThread = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)CallGenerateFunction, buffer, 0, NULL);
}

void VoxelComponent::CallGenerateFunction(LPVOID _buffer)
{
	printf("CallGenerateFunction\n");
	VertAsyncBuffer* buffer = (VertAsyncBuffer*)_buffer;
	std::vector<VertexBuffer> vertices;
	buffer->voxel->UpdateMesh(buffer->isNew);
	//buffer->voxel->BuildVertexBufferGS(buffer->targetDepth,vertices);
	buffer->voxel->SetOctreeDepth(buffer->targetDepth);
	buffer->voxel->SetVertexBuildState(VERT_BUILD_FINISHED);
	delete buffer;
	_endthreadex(0);
}

void VoxelComponent::UpdatePartialMeshAsync(VoxelComponent * voxel, int targetDepth)
{
	if (voxel->vertBuildState == VERT_BUILD_NONE)
	{
		voxel->SetVertexBuildState(VERT_BUILD_LOADING);
		VertAsyncBuffer* buffer = new VertAsyncBuffer;
		buffer->voxel = voxel;
		buffer->targetDepth = targetDepth;
		voxel->T_BuildThread = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)CallGeneratePartialFunction, buffer, 0, NULL);
	}
}

void VoxelComponent::CallGeneratePartialFunction(LPVOID _buffer)
{
	printf("CallGeneratePartialFunction\n");
	VertAsyncBuffer* buffer = (VertAsyncBuffer*)_buffer;
	std::vector<VertexBuffer> vertices;
	for(auto i: buffer->voxel->uSet)
		buffer->voxel->UpdatePartialMesh(i->GetPosition(),true);
	buffer->voxel->uSet.clear();
	//buffer->voxel->BuildVertexBufferGS(buffer->targetDepth,vertices);
	buffer->voxel->SetOctreeDepth(buffer->targetDepth);
	buffer->voxel->SetVertexBuildState(VERT_BUILD_FINISHED);
	delete buffer;
	_endthreadex(0);
}
void VoxelComponent::LoadHeightMapFromRaw(int _width, int _height,int _depth,const char* filename,int startX, int startZ, int endX, int endZ)
{
	if (useOctree)
		NewOctree((startX!=-1&&endX!=-1)?endX-startX+1:_width);
	else NewChunks((startX != -1 && endX != -1) ? endX - startX+1 : _width, _height, (startZ != -1 && endZ != -1) ? endZ - startZ+1 : _depth);

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
	printf("%d BYTE chunk data ���� �Ϸ�\n", width*depth*height);
}

void VoxelComponent::LoadCube(int _width, int _height, int _depth)
{
	if(!useOctree)
		NewChunks(_width, _height, _depth);
	else NewOctree(_width);
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
	if (useOctree)
		NewOctree(_width);
	else NewChunks(_width, _height, _depth);
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
					SetChunk(x, y, z, GetChunk(x, y, z));
				}
			}
		}
	}
	ReleaseChunks();
	printf("Build Octree : %dms\n", GetTickCount64() - tick);
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
