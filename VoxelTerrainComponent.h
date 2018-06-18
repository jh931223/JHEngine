#pragma once
#include <queue>
#include "Component.h"
#include "VoxelComponent.h"
#include <mutex>
class VoxelTerrainComponent :
	public Component
{
public:
	VoxelTerrainComponent();
	~VoxelTerrainComponent();
	static void T_CallUpdateMesh(LPVOID p);
	void T_UpdateMesh();
	void ReserveVoxelUpdate(VoxelComponent* voxel);
	void LateUpdate() override;
	std::mutex mutex;
	std::queue<VoxelComponent*> T_waitForUpdate;
	HANDLE T_UpdateThread;
	bool isUpdating;
};

