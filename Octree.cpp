#pragma once
#include"stdafx.h"
#include"Octree.h"

void OctreeNode::SetChildNode(int index,OctreeNode* child)
{
	childNode[index] = child;
	childNode[index]->parentNode = this;
}

OctreeNode * OctreeNode::FindNode(XMFLOAT3 pos)
{
	if(!IsInCube(pos))
		return NULL;
	OctreeNode* node=NULL;
	if (cellSize==1)
	{
		return this;
	}
	for (int i = 0; i < 8; i++)
	{
		node=childNode[i]->FindNode(pos);
		if (node != NULL)
		{
			return node;
		}
	}
	return NULL;
}

bool OctreeNode::IsInCube(XMFLOAT3 pos)
{
	float halfSize = cellSize * 0.5f;
	if (pos.x <= cellPosition.x + halfSize&&pos.x >= cellPosition.x - halfSize)
		if (pos.y <= cellPosition.y + halfSize&&pos.y >= cellPosition.y - halfSize)
			if (pos.z <= cellPosition.z + halfSize&&pos.z >= cellPosition.z - halfSize)
				return true;
	return false;
}
