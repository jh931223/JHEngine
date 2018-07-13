#include "stdafx.h"
#include "VoxelTerrainComponent.h"
#include <queue>
#include<process.h>
VoxelTerrainComponent::VoxelTerrainComponent()
{
	isUpdating = false;
}


VoxelTerrainComponent::~VoxelTerrainComponent()
{
	if (T_UpdateThread)
		CloseHandle(T_UpdateThread);
}

void VoxelTerrainComponent::T_CallUpdateMesh(LPVOID p)
{
	((VoxelTerrainComponent*)p)->T_UpdateMesh();
}

void VoxelTerrainComponent::T_UpdateMesh()
{
	while(T_waitForUpdate.size()>0)
	{
		T_waitForUpdate.front()->UpdateMesh();
		T_waitForUpdate.pop();
	}
	isUpdating = false;
	_endthreadex(0);
}

void VoxelTerrainComponent::ReserveVoxelUpdate(VoxelComponent * voxel)
{
	if (!isUpdating)
	{
		T_waitForUpdate.push(voxel);
		
	}
}

void VoxelTerrainComponent::LateUpdate()
{
	if (!isUpdating)
	{
		if (T_waitForUpdate.size() > 0)
		{
			T_UpdateThread = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)T_CallUpdateMesh, this, 0, NULL);
			CloseHandle(T_UpdateThread);
			isUpdating = true;
		}
	}
}
