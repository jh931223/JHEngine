#pragma once
#include "Component.h"
#include <vector>
#include "MeshClass.h"
#include <list>
#include "ThreadPool.h"
#include<unordered_map>
#include<map>


//#include"VoxelTable.h"
class MeshRenderer;
class Material;
template<typename T> class Octree;
template<typename T> class OctreeNode;
template<typename T> class ArrayedOctree;



class VoxelComp : public Component
{
public:

	struct VoxelData
	{
		int material = 0;
		float isoValue = -1;
	};

	struct MESH_RESULT
	{
		Mesh* newMesh;
		XMFLOAT3 pos;
		int lodLevel;
	};

	struct INPUT_BUFFER
	{
		int x, y, z;
		int lodLevel;
		short transitionCellBasis;
		void SetXYZ(XMFLOAT3 pos, int _partitionSize)
		{
			float s = 1 / _partitionSize;
			x = pos.x*s;
			y = pos.y*s;
			z = pos.z*s;
		}
		friend bool operator==(const INPUT_BUFFER& data1, const INPUT_BUFFER& data2)
		{
			return  ((data1.x == data2.x) && (data1.y == data2.y) && (data1.z == data2.z)&& (data1.lodLevel==data2.lodLevel));
		}
	};

	VoxelComp();
	virtual ~VoxelComp();
	void UpdateMeshRenderer(Mesh * newMesh,XMFLOAT3 pos, int lodLevel);
	void Update() override;
	void OnStart() override;
	void Initialize();

	unsigned int GetCellIndex(int x, int y, int z);

	bool SetVoxel(int x, int y, int z, VoxelData, bool isInit=false);
	bool EditVoxel(XMFLOAT3 pos, float _radius, float _strength);
	VoxelData GetVoxel(int x, int y, int z);

	VoxelComp::VoxelData GetVoxel(XMFLOAT3 pos);
	int GetPartitionSize() { return partitionSize; }
	XMFLOAT3 GetLastBasePosition() { return lastBasePosition; }
private:
	XMFLOAT3 CalcNormal(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3);
	

	// 복셀큐브 face 생성 메소드
	void CreateCubeFace_Up(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Down(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Right(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Left(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Forward(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Backward(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);

	void PolygonizeCell(XMFLOAT3 pos, int _unit,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices,XMFLOAT3 min=XMFLOAT3(0,0,0),XMFLOAT3 max=XMFLOAT3(1,1,1));
	void PolygonizeCell(XMFLOAT3 pos, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, XMFLOAT3 corners[]);

	void GetVertexInnerBox(short _basis, XMFLOAT3 offset,int _unit, XMFLOAT3 vertOut[]);

	void PolygonizeTransitionCell(XMFLOAT3 pos, int _unit, short _basis, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices);

	MESH_RESULT GenerateCubeFaces(XMFLOAT3 pos);

	MESH_RESULT GeneratePartitionFaces(XMFLOAT3 pos,int lodLevel=0, short transitionCellBasis=0);

	void LoadHeightMapFromRaw(int,int,int,int,const char*,int startX = -1, int startZ = -1, int endX = -1, int endZ = -1);
	void LoadCube(int,int,int);
	void LoadPerlin(int _width,int _height, int _depth, int _maxHeight,float refinement);


	void NewChunks(int _w, int _h, int _d);

	void NewOctree(int _length);
	void ReleaseOctree();

	void SetOctreeDepth(int _targetDepth);


	XMFLOAT2 GetUV(BYTE);

	XMFLOAT3 CovertToChunkPos(XMFLOAT3 targetpos,bool returnNan=true);



	MESH_RESULT CreateNewMesh(INPUT_BUFFER input,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices);
	MESH_RESULT UpdatePartialMesh(XMFLOAT3 pos, int lodLevel = 0, short transitionCellBasis=0);

	void UpdateMesh(int lodLevel=0);
	void UpdateVoxelMesh();
	void UpdateMarchingCubeMesh(int lodLevel = 0);
	XMFLOAT3 GetPositionFromIndex(int index);
	int GetIndexFromPosition(XMFLOAT3 pos);
	void RefreshLODNodes();
	void RefreshLODNodes(XMFLOAT3 centerPos);
	bool FrustumCheckCube(float xCenter, float yCenter, float zCenter, float radius);
	bool FrustumCheckSphere(float xCenter, float yCenter, float zCenter, float radius);
	void ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix);

	void BuildVertexBufferFrustumCulling(int targetDepth);

	void UpdateMeshAsync(int lodLevel);
	short ReserveUpdate(XMFLOAT3 pos, short _basis, short _lodLevel, bool isDeforming, bool checkDuplicated);
	short ReserveUpdate(XMFLOAT3 pos, bool isDeforming = false, bool checkDuplicated = true);

	void ReadRawEX(unsigned char** &_srcBuf, const char* filename, int _width, int _height);
	void ReadRawEX16(unsigned short** &_srcBuf, const char* filename, int _width, int _height, int&, int&);
	XMFLOAT3 GetPartitionCenter(XMFLOAT3 basePos);
	XMFLOAT3 GetPartitionStartPos(XMFLOAT3 basePos);
	int GetLODLevel(const XMFLOAT3& basePos, const XMFLOAT3& targetPos);
	void SetLODLevel(int level, float distance);

	void ProcessLOD();
	void ProcessUpdateQueue();
	void ProcessResultQueue();

	short GetTransitionBasis(int lodLevel, XMFLOAT3 basePos, XMFLOAT3 targetPos);

	XMFLOAT3 lerpSelf(XMFLOAT3 p, XMFLOAT3 target, float acc)
	{
		return p + (target - p)*acc;
	}

	int ReadTXT(const char* filename);

private:

	static const int maxLODLevel = 3;

	int width, height, depth;
	float unit;
	float tUnit;
	int tAmount;

	bool useMarchingCube=true;
	bool useAsyncBuild=true;
	bool useFrustum=false;
	bool useSpartialMatrix=false;
	bool useArrayedOctree = false;
	bool useGPGPU = false;

	int currentOctreeDepth = 0;

	VoxelData* chunks;
	std::unordered_map<unsigned int, VoxelData> spartialMatrix;
	ArrayedOctree<VoxelData>* aOctree;

	Octree<MeshRenderer*>* gOctreeMeshRenderer;

	

	int LODDistance[3]{ 0, };

	OctreeNode<MeshRenderer*>* currentLODNode;

	struct LODGroupData
	{
		short level=0;
		short transitionBasis=0;
	};
	std::unordered_map<int, LODGroupData> lodGropups;


	XMFLOAT3 lastBasePosition;
	XMFLOAT3 customPos;
	bool isLockedBasePosition;

	int isoLevel = 0;
	float strength=0.5f;
	float brushRadius = 3.0f;

	int partitionSize=32;


	ThreadPool<INPUT_BUFFER,MESH_RESULT> threadPool_Main;
	ThreadPool<INPUT_BUFFER,MESH_RESULT> threadPool_Deform;


	std::list<INPUT_BUFFER> updateQueue_Main;
	std::list<INPUT_BUFFER> updateQueue_Deform;

	std::vector<MESH_RESULT> meshBuildResult;
	GameObject* camera;


	//frustum
	XMVECTOR m_planes[6];
	//
};
