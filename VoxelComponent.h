#pragma once
#include "Component.h"
#include <vector>
#include <list>
#include<unordered_map>
#include "ThreadPool.h"
#include"JobSystem.h"
#include"MeshClass.h"
#include"Octree.h"
class MeshRenderer;
class Material;
class RegularCellCache;
class VoxelCollider;
class VoxelComponent;
//#define USE_JOBSYSTEM

struct COMMAND_BUFFER
{
	int x, y, z;
	int lodLevel=-1;
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


class VoxelComponent : public BehaviourComponent
{
public:
	//#pragma pack(push, 1)
	struct VoxelData
	{
		int material = 0;
		float isoValue = -1;
	};
	struct ChunkData
	{
		ChunkData() {}
		void MakeChunk(int length)
		{
			chunk = new VoxelData[length*length*length];
		}
		~ChunkData() { if(chunk)delete[] chunk; }
		bool IsHaveChunk()
		{
			return (chunk);
		}
		VoxelData* chunk = NULL;
		//MeshRenderer* renderer=NULL;
		//bool isPolygonizable = false;
	};	
	struct LODGroupData
	{
		short level = 0;
		short transitionBasis = 0;
	};
	typedef std::vector<OctreeNode<ChunkData>*> ChunkLeafs;
	typedef std::vector<OctreeNode<MeshRenderer*>*> RendererLeafs;

	enum ReserveType
	{
		Reserve_Deform,Reserve_Load
	};
	enum BrushType
	{
		Brush_Default,Brush_Sphere,Brush_Cube
	};

	VoxelComponent();
	~VoxelComponent();
	void Update() override;
	void OnStart() override;
	void Initialize();

	unsigned int GetCellIndex(int x, int y, int z);

	bool SetVoxel(int x, int y, int z, VoxelData, bool isInit=false);
	bool EditVoxel(XMFLOAT3 pos, float _radius, float _strength, BrushType _brushType=Brush_Sphere);
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
	

	// 복셀큐브 face 생성 메소드
	void CreateCubeFace_Up(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Down(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Right(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Left(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Forward(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);
	void CreateCubeFace_Backward(float x, float y, float z, float _unit, BYTE, int&, std::vector<VertexBuffer>&, std::vector<unsigned long>&);

	void PolygonizeRegularCell(XMFLOAT3 pos, XMINT3 offset, int _unit,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, RegularCellCache* cache,XMFLOAT3 min=XMFLOAT3(0,0,0),XMFLOAT3 max=XMFLOAT3(1,1,1));
	void PolygonizeRegularCell(XMFLOAT3 pos, XMINT3 offset, int _unit, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices, RegularCellCache* cache, XMFLOAT3 corners[]);
	void PolygonizeTransitionCell(XMFLOAT3 pos, XMINT3 offset, int _unit, short _basis, std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices);

	void GetVertexInnerBox(short _basis, XMINT3 offset,int _unit, XMFLOAT3 vertOut[]);


	RESULT_BUFFER GenerateCubeFaces(XMFLOAT3 pos);

	RESULT_BUFFER GeneratePartitionFaces(XMFLOAT3 pos,int lodLevel=0, short transitionCellBasis=0);

	void LoadHeightMapFromRaw(int,int,int,int,const char*,int startX = -1, int startZ = -1, int endX = -1, int endZ = -1);

	void GetFileNamesInDirectory(const char* path,std::vector<std::string>& vstr);

	bool IsPolygonizableCell(XMFLOAT3 pos, int _size = 1);

	void LoadCube(int,int,int);
	void LoadPerlin(int _width,int _height, int _depth, int _maxHeight,float refinement);


	void NewChunks(int _w, int _h, int _d);

	void ReleaseChunks();

	XMFLOAT2 GetUV(BYTE);

	XMFLOAT3 CovertToChunkPos(XMFLOAT3 targetpos,bool returnNan=true);



	RESULT_BUFFER CreateNewMesh(COMMAND_BUFFER input,std::vector<VertexBuffer>& vertices, std::vector<unsigned long>& indices);

	void UpdateMesh(int lodLevel=0);
	void UpdateVoxelMesh();
	void UpdateMarchingCubeMesh(int lodLevel = 0);
	XMFLOAT3 GetPositionFromIndex(int index);
	unsigned int GetIndexFromPosition(XMFLOAT3 pos);
	void RefreshLODNodes(XMFLOAT3 centerPos);

	void UpdateMeshAsync(int lodLevel);
	COMMAND_BUFFER ReserveUpdate(XMFLOAT3 pos, short _basis, short _lodLevel, ReserveType reserveType= ReserveType::Reserve_Load , bool isEnableOverWrite=true,bool autoPush=true);
	COMMAND_BUFFER ReserveUpdate(XMFLOAT3 pos, ReserveType reserveType = ReserveType::Reserve_Load, bool isEnableOverWrite = true, bool autoPush = true);

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

	void SetBrush(BrushType _brush);

private:

	static const int maxLODLevel = 3;

	
	struct MapInfo
	{
		int width, height, depth;
		int partitionSize = 32;
	} info;

	float tUnit;
	int tAmount;
	bool useMarchingCube=true;
	bool useAsyncBuild=true;
	bool useGetDataFromFile = false;
	char dataPath[256];
	const char* pathRoot = "MapData";

	

	int LODDistance[maxLODLevel+1]{ 0, };


	std::unordered_map<int, LODGroupData> lodGropups;


	XMFLOAT3 lastBasePosition;
	bool isLockedBasePosition;

	int isoLevel = 0;
	bool keepPress = false;


#ifndef USE_JOBSYSTEM
	ThreadPool<COMMAND_BUFFER,RESULT_BUFFER> threadPool[2];
#endif
	std::list<COMMAND_BUFFER> commandQueue[3];


	std::vector<RESULT_BUFFER> meshBuildResult;
	GameObject* camera;

public:
	bool useOctree = true;
	Octree< ChunkData >* tempChunks;
	std::vector< ChunkData > chunkArray;
	Octree<MeshRenderer*>* meshRendererOctree;
	struct BrushInfo
	{
		BrushType brushType = BrushType::Brush_Sphere;
		float strength = 0.5f;
		float brushRadius = 3.0f;
	}brushInfo;
	MeshRenderer* targetMesh;
};
