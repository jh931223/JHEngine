#include "stdafx.h"
#include "VoxelCollider.h"
#include "BoxCollider.h"
#include "MeshCollider.h"
#include "SphereCollider.h"
#include "MeshClass.h"
#include "VoxelComponent.h"
#include "Octree.h"
#include "MeshRenderer.h"

VoxelCollider::VoxelCollider()
{
}


VoxelCollider::~VoxelCollider()
{
}

std::vector<XMFLOAT3> VoxelCollider::DetectCollisionWithMeshCollider(MeshCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<XMFLOAT3> VoxelCollider::DetectCollisionWithBoxCollider(BoxCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<XMFLOAT3> VoxelCollider::DetectCollisionWithSphereCollider(SphereCollider * col)
{
	return std::vector<XMFLOAT3>();
}

std::vector<RaycastHit> VoxelCollider::DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end)
{
	std::vector<RaycastHit> output;
	std::vector<Mesh*> chunkFindResult;
	GetChunksIncludeLine(start, end, voxel->meshRendererOctree->root, chunkFindResult);
	if (chunkFindResult.size())
	{

		MeshCollider meshChecker(false);
		for(auto i:chunkFindResult)
		{
			meshChecker.mesh = i;
			for (auto j : meshChecker.DetectCollisionWithLine(start, end))
				output.push_back(j);
		}
	}
	return output;
}

Collider::ColliderType VoxelCollider::GetColliderType()
{
	return Collider::ColliderType();
}

void VoxelCollider::GetChunksIncludeLine(XMFLOAT3 _lineStart, XMFLOAT3 _lineEnd, OctreeNode<MeshRenderer*>* _node, std::vector < Mesh*>& result)
{
	if (_node == NULL)
		return;
	BoxCollider boxChecker(false);
	boxChecker.offset = _node->GetCenerPosition();
	float halfSize = _node->GetCellSize()*0.5f;
	boxChecker.extends = XMFLOAT3(halfSize, halfSize, halfSize);
	auto boxResult = boxChecker.DetectCollisionWithLine(_lineStart, _lineEnd);
	if (boxResult.size())
	{
		if (_node->IsLeaf())
		{
			if (_node->GetValue())
				result.push_back(_node->GetValue()->GetMesh());
		}
		else
		{
			for (int i = 0; i < 8; i++)
			{
				GetChunksIncludeLine(_lineStart, _lineEnd, _node->GetChild(i), result);
			}
		}
	}
}
