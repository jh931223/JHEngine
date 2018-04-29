#pragma once
class OctreeNode
{
	float cellSize;
	XMFLOAT3 cellPosition;
	OctreeNode* parentNode;
	OctreeNode* childNode[8];
	void SetChildNode(int,OctreeNode* child);
	OctreeNode* FindNode(XMFLOAT3);
	bool IsInCube(XMFLOAT3);
};
