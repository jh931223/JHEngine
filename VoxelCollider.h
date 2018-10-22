#pragma once
#include "Collider.h"
#include <vector>
class MeshRenderer;
class Mesh;
class VoxelComponent;
template<typename T> class OctreeNode;

class VoxelCollider :
	public Collider
{
public:
	VoxelComponent * voxel;
public:
	VoxelCollider();
	~VoxelCollider();

	// Collider��(��) ���� ��ӵ�
	virtual std::vector<XMFLOAT3> DetectCollisionWithMeshCollider(MeshCollider * col) override;
	virtual std::vector<XMFLOAT3> DetectCollisionWithBoxCollider(BoxCollider * col) override;
	virtual std::vector<XMFLOAT3> DetectCollisionWithSphereCollider(SphereCollider * col) override;
	virtual std::vector<RaycastHit> DetectCollisionWithLine(XMFLOAT3 start, XMFLOAT3 end) override;
	virtual Collider::ColliderType GetColliderType() override;
private:
	void GetChunksIncludeLine(XMFLOAT3 _lineStart, XMFLOAT3 _lineEnd, OctreeNode<MeshRenderer*>* _node, std::vector < Mesh*>& result);
};

