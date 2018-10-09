#pragma once
#include "Component.h"
#include <vector>
#include <list>
#include<unordered_map>
#include "ThreadPool.h"
#include"JobSystem.h"
#include"MeshClass.h"
class MeshRenderer;
class Material;
class VoxelComponent;
template<typename T> class Octree;
template<typename T> class OctreeNode;

struct COMMAND_BUFFER
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
	bool operator==(const COMMAND_BUFFER& data1)
	{
		return  ((data1.x == x) && (data1.y == y) && (data1.z == z));
	}
};
struct RESULT_BUFFER
{
	Mesh* newMesh;
	XMFLOAT3 pos;
	int lodLevel;
};
struct PolygonizeTask : public ITaskParallel
{
	std::vector<COMMAND_BUFFER> commandBuffers;
	std::vector<RESULT_BUFFER> resultBuffers;
	VoxelComponent* component;
protected:
	bool Excute(int index) override;
};
struct SaveTask : public ITaskParallel
{
	std::vector<XMFLOAT3> commandBuffers;
	VoxelComponent* component;
	virtual ~SaveTask()
	{

	}
protected:
	bool Excute(int index) override;
};
class VoxelComponent : public Component
{
public:
	//#pragma pack(push, 1)
	struct VoxelData
	{
		int material = 0;
		float isoValue = -1;
	};
//#pragma pack(pop) 

	

	VoxelComponent();
	~VoxelComponent();
	void Update() override;
	void OnStart() override;
	void Initialize();

	unsigned int GetCellIndex(int x, int y, int z);

	bool SetVoxel(int x, int y, int z, VoxelData, bool isInit=false);
	bool EditVoxel(XMFLOAT3 pos, float _radius, float _strength);
	VoxelData GetVoxel(int x, int y, int z);

	VoxelComponent::VoxelData GetVoxel(XMFLOAT3 pos);
	int GetPartitionSize() { return info.partitionSize; }
	XMFLOAT3 GetLastBasePosition() { return lastBasePosition; }

	void UpdateMeshRenderer(Mesh * newMesh, XMFLOAT3 pos, int lodLevel);
	RESULT_BUFFER UpdatePartialMesh(XMFLOAT3 pos, int lodLevel = 0, short transitionCellBasis = 0);


	void LoadMapData(const char * _path);
	void SaveMapData(const char* _path);
	void LoadMapInfo();
	void InitDataDirectory(const char * _path);
	void SaveMapInfo();
	bool ReadVoxelData(XMFLOAT3 pos, const char* _path = 0);
	void WriteVoxelData(XMFLOAT3 pos);
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

	RESULT_BUFFER GenerateCubeFaces(XMFLOAT3 pos);

	RESULT_BUFFER GeneratePartitionFaces(XMFLOAT3 pos,int lodLevel=0, short transitionCellBasis=0);

	void LoadHeightMapFromRaw(int,int,int,int,const char*,int startX = -1, int startZ = -1, int endX = -1, int endZ = -1);

	void GetFileNamesInDirectory(const char* path,std::vector<std::string>& vstr);


	void LoadCube(int,int,int);
	void LoadPerlin(int _width,int _height, int _depth, int _maxHeight,float refinement);


	void NewChunks(int _w, int _h, int _d);

	void ReleaseChunks();

	void SetOctreeDepth(int _targetDepth);



	XMFLOAT2 GetUV(BYTE);

	XMFLOAT3 CovertToChunkPos(XMFLOAT3 targetpos,bool returnNan=true);



	RESULT_BUFFER CreateNewMesh(COMMAND_BUFFER input,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices);

	void UpdateMesh(int lodLevel=0);
	void UpdateVoxelMesh();
	void UpdateMarchingCubeMesh(int lodLevel = 0);
	XMFLOAT3 GetPositionFromIndex(int index);
	unsigned int GetIndexFromPosition(XMFLOAT3 pos);
	void RefreshLODNodes();
	void RefreshLODNodes(XMFLOAT3 centerPos);
	bool FrustumCheckCube(float xCenter, float yCenter, float zCenter, float radius);
	bool FrustumCheckSphere(float xCenter, float yCenter, float zCenter, float radius);
	void ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix);

	void BuildVertexBufferFrustumCulling(int targetDepth);

	void UpdateMeshAsync(int lodLevel);
	short ReserveUpdate(XMFLOAT3 pos, short _basis, short _lodLevel, bool isDeforming=false , bool isEnableOverWrite=true);
	short ReserveUpdate(XMFLOAT3 pos, bool isDeforming = false, bool isEnableOverWrite = true);

	void ReadRawEX(unsigned char** &_srcBuf, const char* filename, int _width, int _height);
	void ReadRawEX16(unsigned short** &_srcBuf, const char* filename, int _width, int _height, int&, int&);
	XMFLOAT3 GetPartitionCenter(XMFLOAT3 basePos);
	XMFLOAT3 GetPartitionStartPos(XMFLOAT3 basePos);
	int GetLODLevel(const XMFLOAT3& basePos, const XMFLOAT3& targetPos);
	void SetLODLevel(int level, float distance);

	void ProcessLOD();
	void ProcessCommandQueue();
	void ProcessResultQueue();

	short GetTransitionBasis(int lodLevel, XMFLOAT3 basePos, XMFLOAT3 targetPos);

	XMFLOAT3 lerpSelf(XMFLOAT3 p, XMFLOAT3 target, float acc)
	{
		return p + (target - p)*acc;
	}

	int ReadTXT(const char* filename);

private:

	static const int maxLODLevel = 2;

	
	//#pragma pack(push, 1)
	struct MapInfo
	{
		int width, height, depth;
		int partitionSize = 32;
	};
//#pragma pack(pop) 
	MapInfo info;
	float unit;
	float tUnit;
	int tAmount;
	bool useMarchingCube=true;
	bool useAsyncBuild=true;

	bool useFrustum=false;
	bool useGPGPU = false;
	bool useGetDataFromFile = false;

	int currentOctreeDepth = 0;


	char dataPath[256];
	const char* pathRoot = "MapData";
	Octree< std::vector<VoxelData> >* tempChunks;
	Octree<MeshRenderer*>* meshRendererOctree;

	

	int LODDistance[maxLODLevel+1]{ 0, };

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





	ThreadPool<COMMAND_BUFFER,RESULT_BUFFER> threadPool_Main;
	ThreadPool<COMMAND_BUFFER,RESULT_BUFFER> threadPool_Deform;


	std::list<COMMAND_BUFFER> commandQueue_Main;
	std::list<COMMAND_BUFFER> commandQueue_Deform;

	std::vector<RESULT_BUFFER> meshBuildResult;
	GameObject* camera;


	//frustum
	XMVECTOR m_planes[6];
	//
};
