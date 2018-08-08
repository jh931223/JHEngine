#pragma once
#include<vector>
template<typename T> class ArrayedOctree
{
public:
	T * * nodes;
	int depth;
	int size;
	int arraySize;
	float unit = 1.0f;
	ArrayedOctree(float _size)
	{
		int s = _size / 2;
		s *= 2;
		size = s;
		depth = 0;
		while ((s = s >> 1))
		{
			depth++;
		}
		if (depth <= 0)
			return;
		nodes = new T*[depth+1];
		for (int i = 0; i < depth+1; i++)
		{
			s = pow(pow(2, i),3);
			nodes[i] = new T[s];
		}
	}
	~ArrayedOctree()
	{
		if (nodes)
		{
			for (int i = 0; i < depth+1; i++)
			{
				delete[] nodes[i];
			}
			delete[] nodes;
		}
		nodes = 0;
	}
	int GetNodeIDX(DirectX::XMFLOAT3 targetPosition, int targetDepth=-1)
	{
		if (targetPosition.x < 0 || targetPosition.y < 0 || targetPosition.z < 0)
			return -1;
		if (targetPosition.x >= size || targetPosition.y >= size || targetPosition.z >= size)
			return -1;
		if (targetDepth == -1)
			targetDepth = depth;
		
		XMFLOAT3 resultPos = GetNodeXYZ(targetPosition, targetDepth);
		int idx = ((int)resultPos.z << targetDepth * 2) + ((int)resultPos.y << targetDepth) + (int)resultPos.x;
		return idx;
	}
	int GetDepthOfSize(int partitionSize)
	{
		int check = 0;
		while (partitionSize > 1)
		{
			partitionSize *= 0.5f;
			check++;
		}
		return depth - check+1;
	}
	XMFLOAT3 GetNodePosition(int idx, int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		int max_x = pow(2, targetDepth);
		int max_y = pow(2, targetDepth);

		int x = idx % (max_x );
		idx /= (max_x);
		int y = idx % (max_y);
		idx /= (max_y);
		int z = idx;
		int size = GetUnitSize(targetDepth);
		return XMFLOAT3(x*size, y*size, z*size);
	}
	int GetNodeDeltaIDX(int idx, int dx, int dy, int dz, int targetDepth=-1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		return idx + (dz << targetDepth * 2) + (dy << targetDepth) + dx;
	}
	void SetValue(XMFLOAT3 targetPosition, T _value,int targetDepth=-1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		int idx = GetNodeIDX(targetPosition, targetDepth);
		if (idx == -1)
			return;
		nodes[targetDepth][idx] = _value;
	}
	void SetValue(int idx, T _value, int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		if (idx == -1)
			return;
		nodes[targetDepth][idx] = _value;
	}
	void GetNodes(int* size, T*& _nodes, int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		if(size!=NULL)
			*size = pow(pow(2, targetDepth),3);
		_nodes = nodes[targetDepth];
	}
	int ConvertDepthIDX(int idx, int fromDepth, int toDepth)
	{
		return GetNodeIDX(GetNodePosition(idx, fromDepth),toDepth);
	}
	T* GetNodes(int* size, int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		if (size != NULL)
			*size = pow(pow(2, targetDepth), 3);
		return nodes[targetDepth];
	}
	void GetNodesBySize(XMFLOAT3 targetPosition,int& size, T* &result, int partitionSize)
	{
		int check = 0;
		while (partitionSize > 1)
		{
			partitionSize *= 0.5f;
			check++;
		}
		size = pow(pow(2, check),3);
		result = &nodes[depth][ConvertDepthIDX(GetNodeIDX(targetPosition, depth - check),depth-check,depth)];
	}
	T GetValue(XMFLOAT3 targetPosition, int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		int idx = GetNodeIDX(targetPosition, targetDepth);
		if (idx == -1)
			return T();
		return nodes[targetDepth][idx];
	}
	T GetValue(int idx, int targetDepth = -1)
	{
		if (idx == -1)
			return T();
		if (targetDepth == -1)
			targetDepth = depth;
		int id = idx;
		T* v = nodes[targetDepth];
		return nodes[targetDepth][idx];
	}
	XMFLOAT3 GetNodeXYZ(XMFLOAT3 targetPosition, int targetDepth=-1)
	{
		if (targetPosition.x < 0 || targetPosition.y < 0 || targetPosition.z < 0)
			return XMFLOAT3(-1,-1,-1);
		if (targetPosition.x >= size || targetPosition.y >= size || targetPosition.z >= size)
			return XMFLOAT3(-1, -1, -1);
		if (targetDepth == -1)
			targetDepth = depth;
		int unit = GetUnitSize(targetDepth);
		return XMFLOAT3((int)(targetPosition.x / unit),(int)(targetPosition.y / unit),(int)(targetPosition.z / unit));
	}
	XMFLOAT3 GetNodeCenterXYZ(XMFLOAT3 targetPosition, int targetDepth = -1)
	{
		if (targetPosition.x < 0 || targetPosition.y < 0 || targetPosition.z < 0)
			return;
		if (targetPosition.x >= size || targetPosition.y >= size || targetPosition.z >= size)
			return;
		if (targetDepth == -1)
			targetDepth = depth;
		int unit = GetUnitSize(targetDepth);
		int halfUnit = unit * 0.5f;
		return XMFLOAT3((int)(targetPosition.x / unit)+halfUnit, (int)(targetPosition.y / unit)+ halfUnit, (int)(targetPosition.z / unit)+ halfUnit);
	}
	int GetCellIndexOfPosition(XMFLOAT3 target, int depth)
	{
		XMFLOAT3 cellCenter = GetNodeCenterXYZ(target, depth);
		int index = 0;
		index |= (target.y >= cellCenter.y) ? 2 : 0;
		index |= (target.x >= cellCenter.x) ? 4 : 0;
		index |= (target.z >= cellCenter.z) ? 1 : 0;
		return index;
	}
	int GetUnitSize(int targetDepth=-1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		return pow(2, depth - targetDepth);
	}
	int GetLength(int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		return pow(2, targetDepth);
	}
};
