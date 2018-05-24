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
template<typename T> class Octree
{
public:
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
		OctreeNode(XMFLOAT3 pos, float unit,int _depth,OctreeNode* _parent)
		{
			cellPosition = pos;
			cellSize = unit;
			depth = _depth;
			parent = _parent;
		}
		~OctreeNode()
		{
			RemoveChilds();
		}
		void Insert(XMFLOAT3 targetPos, T _value,int LOD_Level=0,bool merge=false)
		{
			Subdivide(this,targetPos, _value, depth- LOD_Level,merge);
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
		static void Subdivide(OctreeNode<T>* node, XMFLOAT3 targetPos, T _value, int _depth = 0, bool merge = false)
		{
			if (node == NULL)
				return;
			int idx = GetIndexOfPosition(targetPos, node->GetPosition());
			if (node->IsLeaf())
			{
				if (_value == 0)
					return;
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
					node->childNodes[i] = new OctreeNode<T>(newPos, newSize, _depth,node);
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
						{
							break;
						}
						if (i == 7)
						{
							return Subdivide(node->parent, targetPos, _value, 0, merge);
						}
					}
				}
				return;
			}
			return Subdivide(node->GetChild(idx),targetPos, _value, _depth - 1, merge);
		}
		void GetLeafs(std::vector<OctreeNode<T>*>& _nodeArray)
		{
			if (IsLeaf())
				_nodeArray.push_back(this);
			else
			{
				for (int i=0;i<8;i++)
				{
					childNodes[i]->GetLeafs(_nodeArray);
				}
			}
		}
		bool IsLeaf()
		{
			return childNodes==NULL;
		}
	};

	OctreeNode<T>* root;
	int depth;
	OctreeNode<T>** nodes;
	int nodeSize = 0;
	float unit;
	int length;
	XMFLOAT3 position;
	Octree(XMFLOAT3 pos, float _size, int _depth,bool _merge=false)
	{
		root = new OctreeNode<T>(pos, _size,_depth,NULL);
		depth = _depth;
		printf("merge : %d \n", _merge);
		OctreeNode<T>::Subdivide(root,pos,0, 0, _merge);
		position = pos;
		unit = _size;
		length = pow(2, depth)*unit;
		//NormalizeNodes();
	}
	~Octree()
	{
		if (root)
			delete root;
		root = 0;
		if (nodes)
		{
			delete[] nodes;
		}
		nodes = 0;
	}
	static int GetIndexOfPosition(XMFLOAT3 target, XMFLOAT3 nodePosition)
	{
		int index=0;
		index |= (target.y >= nodePosition.y) ? 2 : 0;
		index |= (target.x >= nodePosition.x) ? 4 : 0;
		index |= (target.z >= nodePosition.z) ? 1 : 0;
		return index;
	}

	void NormalizeNodes()
	{
		if (nodes)
		{
			delete[] nodes;
		}
		for (int i = 0; i <= depth+1; i++)
		{
			nodeSize += pow(pow(2, i),3);
		}
		nodes = new OctreeNode<T>*[nodeSize];
		for (int i = 0; i < nodeSize; i++)
			nodes[i] = NULL;
	}

	void MergeNodes(OctreeNode<T>* node)
	{
		if (node->GetChild(0))
		{
			byte value= GetChild(0)->GetValue();
			for (int i = 0; i < 8; i++)
			{
				if (!node->GetChild(i)->IsLeaf())
				{
					MergeNodes(node->GetChild(i));
				}
				else if (value == GetChild(i)->GetValue())
				{

				}
			}
		}
	}

	void Insert(XMFLOAT3 targetPosition, T value,int level)
	{
		unsigned int idx = GetNodeIDX(targetPosition, level);
		if (idx >= nodeSize)
		{
			printf("배열 사이즈보다 큼\n");
			return;
		}
		if (!nodes[idx])
		{
			XMFLOAT3 newPos = targetPosition-position;

			float nSize= (depth + 1 - level)*unit;
			float x = (int)(newPos.x / nSize)*nSize;
			float y = (int)(newPos.y / nSize)*nSize;
			float z = (int)(newPos.z / nSize)*nSize;
			float halfSize = nSize * 0.5f;
			newPos.x = x+halfSize;
			newPos.y = y+halfSize;
			newPos.z = z+halfSize;
			nodes[idx] = new OctreeNode<T>(newPos, nSize, level, NULL);
		}
		nodes[idx]->SetValue(value);
	}

	int GetNodeIDX(DirectX::XMFLOAT3 targetPosition, int targetDepth)
	{
		if (targetPosition.x < 0 || targetPosition.y < 0 || targetPosition.z < 0)
			return 0;
		if (targetPosition.x >= length || targetPosition.y >= length || targetPosition.z >= length)
			return 0;
		int x = targetPosition.x*(depth + 1 - targetDepth)*unit;
		int y = targetPosition.y*(depth + 1 - targetDepth)*unit;
		int z = targetPosition.z*(depth + 1 - targetDepth)*unit;
		int idx = (z << targetDepth * 2) + (y << targetDepth) + x;
		return idx;
	}

	OctreeNode<T>* GetNodeOfPosition2(XMFLOAT3 target,int level)
	{
		return nodes[GetNodeIDX(target,  level)];
	}

	T GetValueOfPositionByLevel(XMFLOAT3 target,int level)
	{
		unsigned int idx = GetNodeIDX(target, level);
		if (idx == 0)
			return 0;
		if (!nodes[idx])
			return 0;
		return nodes[idx]->GetValue();
	}

	OctreeNode<T>** GetNormalizedNodes()
	{
		return nodes;
	}
	OctreeNode<T>* GetNodeOfPosition(XMFLOAT3 target, OctreeNode<T>* n=NULL)
	{
		if (n == NULL)
		{
			n = root;
			float unit = root->GetCellSize();
			XMFLOAT3 rootPos = root->GetPosition();
			float halfSize = unit * 0.5f;
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
			float unit = root->GetCellSize();
			XMFLOAT3 rootPos = root->GetPosition();
			float halfSize = unit * 0.5f;
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