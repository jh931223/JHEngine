#pragma once
#include<vector>
template<typename T> class ArrayedOctree
{
public:
	//T *  nodes;
	std::vector<T> nodes;
	int depth;
	int arraySize;
	int arraySize3;
	int unitSize;
	ArrayedOctree(float _size, int _unitSize)
	{
		int lastSize = 1;
		while (lastSize < _size)
		{
			lastSize *= 2;
		}
		int s = lastSize / _unitSize;
		arraySize = s;
		unitSize = _unitSize;
		depth = 0;
		arraySize3 = arraySize * arraySize * arraySize;
		while ((s = s >> 1))
		{
			depth++;
		}
		if (depth <= 0)
			return;
		nodes.resize(arraySize3);
		//nodes = new T[arraySize3];
	}
	ArrayedOctree(float _size) : ArrayedOctree(_size,1)
	{
	}
	~ArrayedOctree()
	{
		//if (nodes)
		//{
		//	delete[] nodes;
		//}
		//nodes = 0;
		nodes.clear();
	}
	unsigned int GetNodeIndex(XMFLOAT3 pos, int _targetDepth = 0)
	{
		unsigned int bit = 0;
		int x = pos.x;
		int y = pos.y;
		int z = pos.z;
		for (int i = _targetDepth; i<depth; i++)
		{
			bit |= ((x >> i) & 1) << i * 3 + 2;
			bit |= ((y >> i) & 1) << i * 3 + 1;
			bit |= ((z >> i) & 1) << i * 3 ;
		}
		return bit;
	}
	XMFLOAT3 GetNodePosition(unsigned int _index)
	{
		int x = 0;
		int y = 0;
		int z = 0;
		for (int i = 0; i<depth; i++)
		{
			x |= ((_index >> i*3+2) & 1) << i;
			y |= ((_index >> i*3+1) & 1) << i;
			z |= ((_Index >> i*3) & 1) << i;
		}
		return XMFLOAT3(x,y,z);
	}
	void SetNode(XMFLOAT3 pos, T value,int _targetDepth=0)
	{
		unsigned int index = GetNodeIndex(pos, _targetDepth);
		if (index >= arraySize3)
			return;
		nodes[index] = value;
	}
	T GetNode(XMFLOAT3 pos, int _targetDepth = 0)
	{
		unsigned int index = GetNodeIndex(pos, _targetDepth);
		if (index >= arraySize3)
			return T();
		return nodes[index];
	}
	T* GetNodePointer(XMFLOAT3 pos, int _targetDepth = 0)
	{
		unsigned int index = GetNodeIndex(pos, _targetDepth);
		if (index >= arraySize3)
			return NULL;
		return &nodes[index];
	}
	std::vector<T>& GetNodesArray()
	{
		return nodes;
	}
	int GetArray3Size()
	{
		return arraySize3;
	}
	int GetArraySize()
	{
		return arraySize;
	}
};
