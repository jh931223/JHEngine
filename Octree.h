#pragma once
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
		XMFLOAT3 cellPosition;
		OctreeNode<T>** childNodes;
		T value;
	public:
		OctreeNode * parent;
		OctreeNode(XMFLOAT3 pos, float size,int _depth,OctreeNode* _parent)
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
		void Insert(XMFLOAT3 targetPos, T _value,int LOD_Level=0)
		{
			Subdivide(targetPos, _value, depth- LOD_Level);
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
		OctreeNode<T>* GetChild(int _index)
		{
			return childNodes[_index];
		}
		void Subdivide(XMFLOAT3 targetPos,T _value,int depth=0)
		{
			int idx = GetIndexOfPosition(targetPos, cellPosition);
			if (childNodes == NULL)
			{
				childNodes = new OctreeNode<T>*[8];
				float newSize = cellSize * 0.5f;
				float newHalfSize = newSize * 0.5f;
				for (int i = 0; i < 8; i++)
				{
					XMFLOAT3 newPos = cellPosition;
					if (i & 2)newPos.y += newHalfSize;
					else newPos.y -= newHalfSize;
					if (i & 4)newPos.x += newHalfSize;
					else newPos.x -= newHalfSize;
					if (i & 1)newPos.z += newHalfSize;
					else newPos.z -= newHalfSize;
					childNodes[i] = new OctreeNode<T>(newPos, newSize,depth,this);
				}
			}
			if (depth > 0)
			{
				childNodes[idx]->Subdivide(targetPos, _value, depth - 1);
			}
			else
			{
				if (!childNodes[idx]->IsLeaf())
					childNodes[idx]->RemoveChilds();
				childNodes[idx]->SetValue(_value);
				//printf("Depth : %d pos : %f %f %f size %f root pos : %f %f %f\n", depth, childNodes[idx]->GetPosition().x, childNodes[idx]->GetPosition().y, childNodes[idx]->GetPosition().z, childNodes[idx]->GetCellSize(), childNodes[idx]->GetCellSize(), childNodes[idx]->parent->GetPosition().x, childNodes[idx]->parent->GetPosition().y, childNodes[idx]->parent->GetPosition().z);
				//if (childNodes[idx]->parent->parent == NULL)
				//	return;
				//printf("Depth : %d pos : %f %f %f size %f root pos : %f %f %f\n", depth, childNodes[idx]->parent->GetPosition().x, childNodes[idx]->parent->GetPosition().y, childNodes[idx]->parent->GetPosition().z, childNodes[idx]->parent->GetCellSize(), childNodes[idx]->parent->GetCellSize(), childNodes[idx]->parent->parent->GetPosition().x, childNodes[idx]->parent->parent->GetPosition().y, childNodes[idx]->parent->parent->GetPosition().z);
			}
		}
		bool IsLeaf()
		{
			return childNodes==NULL;
		}
	};

	OctreeNode<T>* root;
	int depth;
	
	Octree(XMFLOAT3 pos, float size, int _depth)
	{
		root = new OctreeNode<T>(pos, size,_depth,root);
		depth = _depth;
		root->Subdivide(pos,0, 0);
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
		index |= (target.y > nodePosition.y) ? 2 : 0;
		index |= (target.x > nodePosition.x) ? 4 : 0;
		index |= (target.z > nodePosition.z) ? 1 : 0;
		return index;
	}
	T GetValueOfPosition(XMFLOAT3 target)
	{
		float size = root->GetCellSize();
		XMFLOAT3 rootPos = root->GetPosition();
		if (target.x < rootPos.x - size * 0.5f)
			return 0;
		if (target.x > rootPos.x + size * 0.5f)
			return 0;
		if (target.y < rootPos.y - size * 0.5f)
			return 0;
		if (target.y > rootPos.y + size * 0.5f)
			return 0;
		if (target.z < rootPos.z - size * 0.5f)
			return 0;
		if (target.z > rootPos.z + size * 0.5f)
			return 0;
		OctreeNode<T>* n = root;
		
		while (!n->IsLeaf())
		{
			int idx = GetIndexOfPosition(target, n->GetPosition());
			n = n->GetChild(idx);
		}
		return n->GetValue();
	}
};