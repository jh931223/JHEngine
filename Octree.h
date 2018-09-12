#pragma once
#include<vector>
#include"RWLock.h"
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
public:
	float cellSize;
	int depth;
	XMFLOAT3 cellPosition;
	T value;
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
	XMFLOAT3 GetCenerPosition()
	{
		return cellPosition+XMFLOAT3(1,1,1)*GetCellSize()*0.5f;
	}
	T GetValue()
	{
		return value;
	}
	float GetCellSize()
	{
		//return pow(2,depth);
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
	int GetIndexOfPosition(XMFLOAT3 target)
	{
		XMFLOAT3 cellCenter = GetCenerPosition();
		//if (target.x >= GetPosition().x + GetCellSize() || target.y >= GetPosition().y + GetCellSize() || target.z >= GetPosition().z + GetCellSize())
		//	return -1;
		//if (target.x < GetPosition().x || target.y < GetPosition().y || target.z < GetPosition().z )
		//	return -1;
		int index = 0;
		index |= (target.y >= cellCenter.y) ? 2 : 0;
		index |= (target.x >= cellCenter.x) ? 4 : 0;
		index |= (target.z >= cellCenter.z) ? 1 : 0;
		return index;
	}
	void GetLeafs(std::vector<OctreeNode<T>*>& _nodeArray,int targetDepth=0)
	{
		if (IsLeaf()||depth==targetDepth)
			_nodeArray.push_back(this);
		else
		{
			for (int i = 0; i<8; i++)
			{
				childNodes[i]->GetLeafs(_nodeArray,targetDepth);
			}
		}
	}
	//void SetChildAtPosition(XMFLOAT3 target,OctreeNode<T>* _c)
	//{
	//	if (IsLeaf())
	//		Subdivide(target, depth - 1, true);
	//	int idx = GetIndexOfPosition(target);
	//	SetChild(idx, _c);
	//}
	void SetParent(OctreeNode<T>* _p)
	{
		parent = _p;
	}
	void SetChild(int idx, OctreeNode<T>* _c)
	{
		if (idx < 0 || idx>7)
			return;
		if(childNodes[idx])
			delete childNodes[idx];
		childNodes[idx] = _c;
		_c->SetParent(this);
	}
	bool IsLeaf()
	{
		return (childNodes == NULL);
	}
};

template<typename T> class Octree
{
public:
	

	OctreeNode<T>* root;
	RWLock rwLock;
	Octree(float _size)
	{
		int lastSize = 1;
		while (lastSize < _size)
		{
			lastSize *= 2;
		}
		int depth = 0;
		int s = lastSize;
		while ((s = s >> 1))
		{
			depth++;
		}
		root = new OctreeNode<T>(XMFLOAT3(0,0,0), pow(2,depth),depth,NULL);
	}
	Octree(float _size,float unitSize)
	{
		int lastSize = 1;
		while (lastSize < _size)
		{
			lastSize *= 2;
		}
		int depth = 0;
		int s = lastSize /unitSize;
		while ((s = s >> 1))
		{
			depth++;
		}
		root = new OctreeNode<T>(XMFLOAT3(0, 0, 0), lastSize, depth, NULL);
	}
	~Octree()
	{
		if (root)
			delete root;
		root = 0;
	}
	void Insert(XMFLOAT3 targetPos, T value, int targetDepth)
	{
		OctreeNode<T>* node = Subdivide(root,targetPos, targetDepth,true);
		node->SetValue(_value);
	}
	OctreeNode<T>* GetNodeAtPosition(XMFLOAT3 targetPos,int targetDepth, OctreeNode<T>* _startNode=NULL)
	{
		if(_startNode)
			return Subdivide(_startNode,targetPos, targetDepth);
		return Subdivide(root,targetPos, targetDepth);
	}
	int GetRootDepth()
	{
		return root->GetDepth();
	}
	void Expand()
	{
		int depth=root->GetDepth()+1;
		OctreeNode<T>* newRoot;
		newRoot = new OctreeNode<T>(XMFLOAT3(0, 0, 0), pow(2, depth), depth, NULL);
		newRoot->SetChild(0, root);
		root = newRoot;
	}
	OctreeNode<T>* GetNodeBySize(XMFLOAT3 pos,int partialSize)
	{
		OctreeNode<T>* node;
		node = GetNodeAtPosition(pos, 0);
		while (node->parent != NULL)
		{
			if (node->GetCellSize() < partialSize)
				node = node->parent;
			else
				break;
		}
		return node;
	}
	OctreeNode<T>* Subdivide(XMFLOAT3 targetPos, int _targetDepth, bool makeChild = false)
	{
		return Subdivide(root, targetPos, _targetDepth, makeChild);
	}
	OctreeNode<T>* Subdivide(OctreeNode<T>* node, XMFLOAT3 targetPos, int _targetDepth, bool makeChild = false)
	{
		//rwLock.EnterReadLock();
		if (_targetDepth == node->depth || node->depth == 0)
		{
			//rwLock.LeaveReadLock();
			return node;
		}
		int idx = node->GetIndexOfPosition(targetPos);
		if (node->IsLeaf())
		{
			if (!makeChild)
			{
				//rwLock.LeaveReadLock();
				return node;
			}
			//rwLock.LeaveReadLock();
			//rwLock.EnterWriteLock();
			node->childNodes = new OctreeNode<T>*[8];
			float newSize = node->GetCellSize() * 0.5f;
			for (int i = 0; i < 8; i++)
			{
				XMFLOAT3 newPos = node->GetPosition();
				if (i & 2)newPos.y += newSize;
				if (i & 4)newPos.x += newSize;
				if (i & 1)newPos.z += newSize;
				node->childNodes[i] = new OctreeNode<T>(newPos, newSize, node->depth - 1, node);
			}
			//rwLock.LeaveWriteLock();
		}
		//else rwLock.LeaveReadLock();
		return Subdivide(node->childNodes[idx], targetPos, _targetDepth, makeChild);
	}


};
