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
	byte idx;
	byte isFull;
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
	OctreeNode<T>* Insert(XMFLOAT3 targetPos, T _value, int LOD_Level = 0)
	{
		return SubdivideThenSet(this, targetPos, _value, depth - LOD_Level);
	}
	void SetValue(T _value)
	{
		value = _value;
		//if (parent != NULL)
		//{
		//	if (_value)
		//	{
		//		parent->isFull |= idx;
		//		if(parent->isFull==255)
		//			parent->SetValue(_value);
		//	}
		//	else
		//	{
		//		parent->isFull &= ~(idx);
		//		if (parent->isFull == 0)
		//			parent->SetValue(_value);
		//	}
		//}
		//if (parent != NULL)
		//{
		//	int numsOfValidChild = 0;
		//	for (int i = 0; i < 8; i++)
		//		if (parent->GetChild(i)->GetValue())
		//			numsOfValidChild++;
		//	if(numsOfValidChild==8)
		//		parent->SetValue(_value);
		//	else
		//		parent->SetValue(0);
		//}
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
	XMFLOAT3 GetStartPosition()
	{
		return cellPosition-XMFLOAT3(1,1,1)*cellSize*0.5f;
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
		return isFull==255;
	}
	static OctreeNode<T>* SubdivideThenSet(OctreeNode<T>* node, XMFLOAT3 targetPos, T _value, int _depth = 0)
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
				node->childNodes[i]->idx = pow(2, i);
			}
		}
		if (_depth == 0)
		{
			if (!node->GetChild(idx)->IsLeaf())
				node->GetChild(idx)->RemoveChilds();
			node->GetChild(idx)->SetValue(_value);
			return node->GetChild(idx);
		}
		return SubdivideThenSet(node->GetChild(idx), targetPos, _value, _depth - 1);
	}
	static OctreeNode<T>* Subdivide(OctreeNode<T>* node, XMFLOAT3 targetPos, int _depth = 0)
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
				node->childNodes[i]->idx = pow(2, i);
			}
		}
		if (_depth == 0)
		{
			if (!node->GetChild(idx)->IsLeaf())
				node->GetChild(idx)->RemoveChilds();
			return node->GetChild(idx);
		}
		return Subdivide(node->GetChild(idx), targetPos, _depth - 1);
	}
	void GetLeafs(std::vector<OctreeNode<T>*>& _nodeArray,int _depth=0)
	{
		if (IsLeaf() || depth == _depth)
			_nodeArray.push_back(this);
		else
		{
			if (IsFull())
			{
				_nodeArray.push_back(this);
				return;
			}
			for (int i = 0; i<8; i++)
			{
				childNodes[i]->GetLeafs(_nodeArray, _depth);
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
	Octree(XMFLOAT3 pos, float _size, int _depth)
	{
		root = new OctreeNode<T>(pos, _size,_depth,NULL);
		depth = _depth;
		OctreeNode<T>::SubdivideThenSet(root,pos,0, 0);
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
	void Insert(XMFLOAT3 targetPos, T _value, int LOD_Level = 0)
	{
		OctreeNode<T>::SubdivideThenSet(root, targetPos, _value, depth - LOD_Level);
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
