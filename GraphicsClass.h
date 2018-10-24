#pragma once
#include<vector>
#include<mutex>
#include"JobSystem.h"
/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = false;
const float SCREEN_DEPTH = 10000.0f;
const float SCREEN_NEAR = 1.0f;
const int SHADOWMAP_WIDTH = 1024;
const int SHADOWMAP_HEIGHT = 1024;
const float SHADOWMAP_DEPTH = 1024.0f;
const float SHADOWMAP_NEAR = 1.0f;


class D3DClass;
class CameraComponent;
class LightComponent;
class MeshRenderer;
class BitmapRenderer;
class Material;
class Mesh;
class RenderTextureClass;

class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();
	D3DClass* GetD3D();

	void PushRenderer(MeshRenderer* renderer);
	void PushBitmapRenderer(BitmapRenderer * renderer);
	void PushLights(LightComponent* renderer);
	void PushCameras(CameraComponent* renderer);
	void SortCameras();
	void RemoveRenderer(MeshRenderer* renderer);
	void RemoveBitmapRenderer(BitmapRenderer * renderer);
	void RemoveLights(LightComponent* renderer);
	void RemoveCameras(CameraComponent* renderer);
	CameraComponent* GetMainCamera();
	bool RenderScene(XMMATRIX viewMatrix, XMMATRIX projectionMatrix, Material* customMaterial = NULL);
	XMFLOAT2 GetScreenSize();
	ID3D11Device* GetDevice();
private:
	bool Render();
	bool RenderScene(CameraComponent* camera,Material* customMaterial=NULL);
	bool RenderCanvas(CameraComponent* m_Camera, Material* customMaterial = NULL);
	bool RenderToTexture(RenderTextureClass * m_RenderTexture, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, Material* customMaterial = NULL);
	bool RenderToTexture(CameraComponent* camera, Material* customMaterial = NULL);

private:
	std::mutex renderMutex;
	D3DClass* m_Direct3D = nullptr;
	std::vector<MeshRenderer*> meshRenderers;
	std::vector<BitmapRenderer*> bitmapRenderers;
	std::vector<LightComponent*> lights;
	std::vector<CameraComponent*> cameras;
	bool useMultiThreadedRendering=false;

	RenderTextureClass* shadowMap;
	Material* shadowMapMaterial;

	Mesh* skyMesh;

	float screenWidth, screenHeight;
};
struct RenderTask : ITaskParallel
{
	std::vector<MeshRenderer*>* renderers;
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	D3DClass* m_Direct3D = NULL;


	// ITaskParallel을(를) 통해 상속됨
	virtual bool Excute(int index);
	virtual void OnFinish(int id)override;
};