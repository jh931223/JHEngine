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
		OctreeNode(XMFLOAT3 pos, float size)
		{
			cellPosition = pos;
			cellSize = size;
		}
		~OctreeNode()
		{
			if (childNodes)
			{
				for (int i = 0; i < 8; i++)
					delete childNodes[i];
				delete[] childNodes;
			}
		}
		void Insert(T _value, XMFLOAT3 targetPos)
		{
			Subdivide(targetPos, _value, depth);
		}
		void SetValue(T _value)
		{
			value = _value;
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
					if (i & 2 == 2)newPos.y += newHalfSize;
					else newPos.y -= newHalfSize;
					if (i & 4 == 4)newPos.x += newHalfSize;
					else newPos.x -= newHalfSize;
					if (i & 1 == 1)newPos.z += newHalfSize;
					else newPos.z -= newHalfSize;
					childNodes[i] = new OctreeNode<T>(newPos, newSize);
				}
			}
			if (depth > 0)
			{
				childNodes[idx]->Subdivide(targetPos, _value, depth - 1);
			}
			else
			{
				childNodes[idx]->SetValue(_value);
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
		root = new OctreeNode<T>(pos, size);
		depth = _depth;
		root->Subdivide(pos,0, 0);
	}
	~Octree()
	{
		if (root)
			delete root;
	}
	static int GetIndexOfPosition(XMFLOAT3 target, XMFLOAT3 nodePosition)
	{
		int index=0;
		index |= (target.y > nodePosition.y) ? 2 : 0;
		index |= (target.x > nodePosition.x) ? 4 : 0;
		index |= (target.z > nodePosition.z) ? 1 : 0;
		return index;
	}
};