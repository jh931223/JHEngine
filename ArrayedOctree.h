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
		nodes = new T*[depth + 1];
		for (int i = 0; i < depth + 1; i++)
		{
			s = pow(pow(2, i),3);
			nodes[i] = new T[s];
		}
	}
	~ArrayedOctree()
	{
		if (nodes)
		{
			for (int i = 0; i < depth + 1; i++)
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
		int x, y, z;
		GetNodeXYZ(targetPosition, x, y, z, targetDepth);
		int idx = (z << targetDepth * 2) + (y << targetDepth) + x;
		return idx;
	}
	XMFLOAT3 GetNodePosition(int idx, int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		int max_x = pow(2, targetDepth);
		int max_y = pow(2, targetDepth);

		int x = idx % (max_x + 1);
		idx /= (max_x + 1);
		int y = idx % (max_y + 1);
		idx /= (max_y + 1);
		int z = idx;
		return XMFLOAT3(x, y, z);
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
	void GetNodes(int* size, T*& _nodes, int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		if(size!=NULL)
			*size = pow(pow(2, targetDepth),3);
		_nodes = nodes[targetDepth];
	}
	T* GetNodes(int* size, int targetDepth = -1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		if (size != NULL)
			*size = pow(pow(2, targetDepth), 3);
		return nodes[targetDepth];
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
	void GetNodeXYZ(XMFLOAT3 targetPosition,int&x,int&y,int&z, int targetDepth=-1)
	{
		if (targetPosition.x < 0 || targetPosition.y < 0 || targetPosition.z < 0)
			return;
		if (targetPosition.x >= size || targetPosition.y >= size || targetPosition.z >= size)
			return;
		if (targetDepth == -1)
			targetDepth = depth;
		float d = targetDepth / depth;
		x = targetPosition.x*d;
		y = targetPosition.y*d;
		z = targetPosition.z*d;
	}
	int GetUnitSize(int targetDepth=-1)
	{
		if (targetDepth == -1)
			targetDepth = depth;
		return pow(2, depth - targetDepth);
	}
};
