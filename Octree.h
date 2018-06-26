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
		return cellPosition+XMFLOAT3(1,1,1)*cellSize*0.5f;
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
	int GetIndexOfPosition(XMFLOAT3 target)
	{
		int index = 0;
		XMFLOAT3 cellCenter = GetCenerPosition();
		index |= (target.y >= cellCenter.y) ? 2 : 0;
		index |= (target.x >= cellCenter.x) ? 4 : 0;
		index |= (target.z >= cellCenter.z) ? 1 : 0;
		return index;
	}
	OctreeNode<T>* Subdivide(XMFLOAT3 targetPos,int _targetDepth,bool makeChild=false)
	{
		if (_targetDepth == depth || depth == 0)
			return this;
		int idx = GetIndexOfPosition(targetPos);
		if(IsLeaf())
		{
			if (!makeChild)
				return this;
			childNodes = new OctreeNode<T>*[8];
			float newSize = GetCellSize() * 0.5f;
			for (int i = 0; i < 8; i++)
			{
				XMFLOAT3 newPos = GetPosition();
				if (i & 2)newPos.y += newSize;
				if (i & 4)newPos.x += newSize;
				if (i & 1)newPos.z += newSize;
				childNodes[i] = new OctreeNode<T>(newPos, newSize, depth-1, this);
			}
		}
		return childNodes[idx]->Subdivide(targetPos, _targetDepth,makeChild);
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
	void SetChildAtPosition(XMFLOAT3 target,OctreeNode<T>* _c)
	{
		if (IsLeaf())
			Subdivide(target, depth - 1, true);
		int idx = GetIndexOfPosition(target);
		SetChild(idx, _c);
	}
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
	Octree(float _size)
	{
		int depth=0;
		int s = _size;
		while ((s = s >> 1))
		{
			depth++;
		}
		root = new OctreeNode<T>(XMFLOAT3(0,0,0), pow(2,depth),depth,NULL);
	}
	~Octree()
	{
		if (root)
			delete root;
		root = 0;
	}
	void Insert(XMFLOAT3 targetPos, T value, int targetDepth)
	{
		OctreeNode<T>* node = root->Subdivide(targetPos, targetDepth,true);
		node->SetValue(_value);
	}
	OctreeNode<T>* GetNodeAtPosition(XMFLOAT3 targetPos,int targetDepth, OctreeNode<T>* _startNode=NULL)
	{
		if(_startNode)
			return _startNode->Subdivide(targetPos, targetDepth);
		return root->Subdivide(targetPos, targetDepth);
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
	OctreeNode<T>* GetPartialNode(XMFLOAT3 pos,int partialSize)
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
};
