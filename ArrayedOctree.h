#pragma once
#include<vector>
enum AONodeIndex
{
	BottomLeftBackward = 0, //000
	BottomLeftForward = 1, //001
	BottomRightBackward = 4, //100
	BottomRightForward = 5, //101
	UpperLeftBackward = 2, //010
	UpperLeftForward = 3, //011
	UpperRightBackward = 6, //110
	UpperRightForward = 7, //111
};
template<typename T>class AONode
{
	float cellSize;
	int depth;
	bool isFull;
	XMFLOAT3 cellPosition;
	T value;
public:
	AONode<T>** childNodes;
	AONode * parent;
	AONode(XMFLOAT3 pos, float size, int _depth, AONode* _parent)
	{
		cellPosition = pos;
		cellSize = size;
		depth = _depth;
		parent = _parent;
	}
	~AONode()
	{
		RemoveChilds();
	}
	AONode<T>* Insert(XMFLOAT3 targetPos, T _value, int LOD_Level = 0, bool merge = false)
	{
		return SubdivideThenSet(this, targetPos, _value, depth - LOD_Level, merge);
	}
	void SetValue(T _value)
	{
		value = _value;
	}
	void RemoveChilds()
	{
		if (childNodes)
		{
			for (int i = 0; i < 8; i++)
				delete childNodes[i];
			delete[] childNodes;
		}
		childNodes = 0;
	}
	XMFLOAT3 GetPosition()
	{
		return cellPosition;
	}
	T GetValue()
	{
		return value;
	}
	float GetCellSize()
	{
		return cellSize;
	}
	unsigned int GetDepth()
	{
		return depth;
	}
	AONode<T>* GetChild(int _index)
	{
		if (!childNodes)
			return NULL;
		return childNodes[_index];
	}
	AONode<T>** GetChilds()
	{
		return childNodes;
	}
	bool IsFull()
	{
		if (GetValue())
			return true;
		if (childNodes)
		{
			for (int i = 0; i < 8; i++)
			{
				if (!childNodes[i]->IsFull())
				{
					break;
				}
			}
		}
		return false;
	}
	static AONode<T>* SubdivideThenSet(AONode<T>* node, XMFLOAT3 targetPos, T _value, int _depth = 0, bool merge = false)
	{
		if (node == NULL)
		{
			return NULL;
		}
		int idx = ArrayedOctree<T>::GetIndexOfPosition(targetPos, node->GetPosition());
		if (node->IsLeaf())
		{
			if (_value == 0)
				return NULL;
			node->childNodes = new AONode<T>*[8];
			float newSize = node->GetCellSize() * 0.5f;
			float newHalfSize = newSize * 0.5f;
			for (int i = 0; i < 8; i++)
			{
				XMFLOAT3 newPos = node->GetPosition();
				if (i & 2)newPos.y += newHalfSize;
				else newPos.y -= newHalfSize;
				if (i & 4)newPos.x += newHalfSize;
				else newPos.x -= newHalfSize;
				if (i & 1)newPos.z += newHalfSize;
				else newPos.z -= newHalfSize;
				node->childNodes[i] = new AONode<T>(newPos, newSize, _depth, node);
			}
		}
		if (_depth == 0)
		{
			if (!node->GetChild(idx)->IsLeaf())
				node->GetChild(idx)->RemoveChilds();
			node->GetChild(idx)->SetValue(_value);
			if (merge)
			{
				for (int i = 0; i < 8; i++)
				{
					if (node->GetChild(i)->GetValue() != _value)
						break;
					if (i == 7)
					{
						if (!node->parent)
						{
							node->RemoveChilds();
							node->SetValue(_value);
							return node;
						}
						return SubdivideThenSet(node->parent, targetPos, _value, 0, merge);
					}
				}
			}
			return node->GetChild(idx);
		}
		return SubdivideThenSet(node->GetChild(idx), targetPos, _value, _depth - 1, merge);
	}
	static AONode<T>* Subdivide(AONode<T>* node, XMFLOAT3 targetPos, int _depth = 0, bool merge = false)
	{
		if (node == NULL)
		{
			return NULL;
		}
		int idx = ArrayedOctree<T>::GetIndexOfPosition(targetPos, node->GetPosition());
		if (node->IsLeaf())
		{
			node->childNodes = new AONode<T>*[8];
			float newSize = node->GetCellSize() * 0.5f;
			float newHalfSize = newSize * 0.5f;
			for (int i = 0; i < 8; i++)
			{
				XMFLOAT3 newPos = node->GetPosition();
				if (i & 2)newPos.y += newHalfSize;
				else newPos.y -= newHalfSize;
				if (i & 4)newPos.x += newHalfSize;
				else newPos.x -= newHalfSize;
				if (i & 1)newPos.z += newHalfSize;
				else newPos.z -= newHalfSize;
				node->childNodes[i] = new AONode<T>(newPos, newSize, _depth, node);
			}
		}
		if (_depth == 0)
		{
			if (!node->GetChild(idx)->IsLeaf())
				node->GetChild(idx)->RemoveChilds();
			return node->GetChild(idx);
		}
		return Subdivide(node->GetChild(idx), targetPos, _depth - 1, merge);
	}
	void GetLeafs(std::vector<AONode<T>*>& _nodeArray)
	{
		if (IsLeaf())
			_nodeArray.push_back(this);
		else
		{
			for (int i = 0; i<8; i++)
			{
				childNodes[i]->GetLeafs(_nodeArray);
			}
		}
	}
	bool IsLeaf()
	{
		return childNodes == NULL;
	}
};

template<typename T> class ArrayedOctree
{
public:
	T** nodes;
	int depth;
	int size;
	int arraySize;
	float unit = 1.0f;
	XMFLOAT3 position;
	ArrayedOctree(XMFLOAT3 pos, float _size)
	{
		int s = _size / 2;
		s *= 2;
		size = s;
		depth=0;
		while ((s=s>>1))
		{
			depth++;
		}
		depth--;
		if (depth<=0)
			return;

		nodes = new T[depth+1];
		for (int i = 0; i < depth+1; i++)
		{
			s = pow(8, i);
			nodes[i] = new T[s];
		}
	}
	~ArrayedOctree()
	{
		if (root)
			delete root;
		root = 0;
	}
	int GetNodeIDX(DirectX::XMFLOAT3 targetPosition, int targetDepth)
	{
		if (targetPosition.x < position.x || targetPosition.y < position.y || targetPosition.z < position.z)
			return -1;
		if (targetPosition.x >= position.x + size || targetPosition.y >= position.y + size || targetPosition.z >= position.z + size)
			return -1;
		int x, y, z;
		GetStartPoint(targetPosition, targetDepth, x, y, z);
		int idx = (z << targetDepth * 2) + (y << targetDepth) + x;
		return idx;
	}
	int GetNodeDeltaIDX(int idx,int targetDepth,int dx, int dy, int dz)
	{
		return idx + (dz << targetDepth * 2) + (dy << targetDepth) + dx;
	}
	void SetValue(XMFLOAT3 targetPosition, int targetDepth,T _value)
	{
		int idx = GetNodeIDX(targetPosition, targetDepth);
		if (idx == -1)
			return;
		nodes[targetDepth][idx] = _value;
	}
	T GetValue(XMFLOAT3 targetPosition, int targetDepth)
	{
		int idx = GetNodeIDX(targetPosition, targetDepth);
		if (idx == -1)
			return;
		return nodes[targetDepth][idx];
	}
	void GetStartPoint(XMFLOAT3 targetPosition, int targetDepth,int&x,int&y,int&z)
	{
		if (targetPosition.x < position.x || targetPosition.y < position.y || targetPosition.z < position.z)
			return -1;
		if (targetPosition.x >= position.x + size || targetPosition.y >= position.y + size || targetPosition.z >= position.z + size)
			return -1;
		float d = targetDepth / depth;
		int x = targetPosition.x*d;
		int y = targetPosition.y*d;
		int z = targetPosition.z*d;
	}
	int GetUnitSize(int targetDepth)
	{
		return pow(2, depth - targetDepth);
	}
	void GetChilds(XMFLOAT3 targetPos, int targetDepth, int* indices)
	{
		int x, y, z, s;
		GetStartPoint(targetPos, targetDepth,x,y,z);
		s=GetUnitSize(targetDepth)*0.5f;
		indices[0] = GetNodeIDX(XMFLOAT3(x + s, y + s, z + s));
		indices[1] = GetNodeIDX(XMFLOAT3(x + s, y + s, z + s));
		indices[2] = GetNodeIDX(XMFLOAT3(x + s, y + s, z + s));
		indices[3] = GetNodeIDX(XMFLOAT3(x + s, y + s, z + s));

		indices[3] = GetNodeIDX(XMFLOAT3(x + s, y + s, z + s));
		indices[3] = GetNodeIDX(XMFLOAT3(x + s, y + s, z + s));
		indices[3] = GetNodeIDX(XMFLOAT3(x + s, y + s, z + s));
		indices[3] = GetNodeIDX(XMFLOAT3(x + s, y + s, z + s));
	}
};
