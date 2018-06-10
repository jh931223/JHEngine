#pragma once
#include<vector>
enum OctreeIndex
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
template<typename T>class OctreeNode
{
	float cellSize;
	int depth;
	bool isFull;
	XMFLOAT3 cellPosition;
	T value;
public:
	OctreeNode<T>** childNodes;
	OctreeNode * parent;
	OctreeNode(XMFLOAT3 pos, float size, int _depth, OctreeNode* _parent)
	{
		cellPosition = pos;
		cellSize = size;
		depth = _depth;
		parent = _parent;
	}
	~OctreeNode()
	{
		RemoveChilds();
	}
	OctreeNode<T>* Insert(XMFLOAT3 targetPos, T _value, int LOD_Level = 0, bool merge = false)
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
	OctreeNode<T>* GetChild(int _index)
	{
		if (!childNodes)
			return NULL;
		return childNodes[_index];
	}
	OctreeNode<T>** GetChilds()
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
	static OctreeNode<T>* SubdivideThenSet(OctreeNode<T>* node, XMFLOAT3 targetPos, T _value, int _depth = 0, bool merge = false)
	{
		if (node == NULL)
		{
			return NULL;
		}
		int idx = Octree<T>::GetIndexOfPosition(targetPos, node->GetPosition());
		if (node->IsLeaf())
		{
			if (_value == 0)
				return NULL;
			node->childNodes = new OctreeNode<T>*[8];
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
				node->childNodes[i] = new OctreeNode<T>(newPos, newSize, _depth, node);
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
	static OctreeNode<T>* Subdivide(OctreeNode<T>* node, XMFLOAT3 targetPos, int _depth = 0, bool merge = false)
	{
		if (node == NULL)
		{
			return NULL;
		}
		int idx = Octree<T>::GetIndexOfPosition(targetPos, node->GetPosition());
		if (node->IsLeaf())
		{
			node->childNodes = new OctreeNode<T>*[8];
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
				node->childNodes[i] = new OctreeNode<T>(newPos, newSize, _depth, node);
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
	void GetLeafs(std::vector<OctreeNode<T>*>& _nodeArray)
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

template<typename T> class Octree
{
public:
	

	OctreeNode<T>* root;
	int depth;
	int size;
	XMFLOAT3 position;
	Octree(XMFLOAT3 pos, float _size, int _depth,bool _merge=false)
	{
		root = new OctreeNode<T>(pos, _size,_depth,NULL);
		depth = _depth;
		OctreeNode<T>::SubdivideThenSet(root,pos,0, 0, _merge);
		position = pos;
		size = _size;
	}
	~Octree()
	{
		if (root)
			delete root;
		root = 0;
	}
	static int GetIndexOfPosition(XMFLOAT3 target, XMFLOAT3 nodePosition)
	{
		int index=0;
		index |= (target.y >= nodePosition.y) ? 2 : 0;
		index |= (target.x >= nodePosition.x) ? 4 : 0;
		index |= (target.z >= nodePosition.z) ? 1 : 0;
		return index;
	}
	void Insert(XMFLOAT3 targetPos, T _value, int LOD_Level = 0, bool merge = false)
	{
		OctreeNode<T>::SubdivideThenSet(root, targetPos, _value, depth - LOD_Level, merge);
	}
	OctreeNode<T>* GetNodeOfPosition(XMFLOAT3 target, OctreeNode<T>* n=NULL)
	{
		if (n == NULL)
		{
			n = root;
			float size = root->GetCellSize();
			XMFLOAT3 rootPos = root->GetPosition();
			float halfSize = size * 0.5f;
			if (target.x < rootPos.x - halfSize)
				return NULL;
			if (target.x >= rootPos.x + halfSize)
				return NULL;
			if (target.y < rootPos.y - halfSize)
				return NULL;
			if (target.y >= rootPos.y + halfSize)
				return NULL;
			if (target.z < rootPos.z - halfSize)
				return NULL;
			if (target.z >= rootPos.z + halfSize)
				return NULL;
		}
		if (n->IsLeaf())
		{
			return n;
		}
		int idx = GetIndexOfPosition(target, n->GetPosition());
		n = n->GetChild(idx);
		return GetNodeOfPosition(target,n);
	}
	T GetValueOfPosition(XMFLOAT3 target,int _depth=0, OctreeNode<T>* n = NULL)
	{
		if (n == NULL)
		{
			n = root;
			float size = root->GetCellSize();
			XMFLOAT3 rootPos = root->GetPosition();
			float halfSize = size * 0.5f;
			if (target.x < rootPos.x - halfSize)
				return NULL;
			if (target.x >= rootPos.x + halfSize)
				return NULL;
			if (target.y < rootPos.y - halfSize)
				return NULL;
			if (target.y >= rootPos.y + halfSize)
				return NULL;
			if (target.z < rootPos.z - halfSize)
				return NULL;
			if (target.z >= rootPos.z + halfSize)
				return NULL;
		}
		if (n->IsLeaf() || n->GetDepth()<= _depth)
		{
			return n->GetValue();
		}
		int idx = GetIndexOfPosition(target, n->GetPosition());
		n = n->GetChild(idx);
		return GetValueOfPosition(target, _depth,n);
	}
	
};
