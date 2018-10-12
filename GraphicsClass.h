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
const int SHADOWMAP_WIDTH = 1600;
const int SHADOWMAP_HEIGHT = 900;


class D3DClass;
class CameraComponent;
class LightComponent;
class MeshRenderer;
class BitmapClass;
class Material;
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
	void PushLights(LightComponent* renderer);
	void PushCameras(CameraComponent* renderer);
	void SortCameras();
	void RemoveRenderer(MeshRenderer* renderer);
	void RemoveLights(LightComponent* renderer);
	void RemoveCameras(CameraComponent* renderer);
	CameraComponent* GetMainCamera();
private:
	bool Render();
	bool RenderScene(CameraComponent* camera,Material* customMaterial);
	bool RenderCanvas(CameraComponent* camera);
	bool RenderToTexture(CameraComponent* camera);

private:
	std::mutex renderMutex;
	D3DClass* m_Direct3D = nullptr;
	BitmapClass* m_Bitmap = nullptr;
	std::vector<MeshRenderer*> meshRenderers;
	std::vector<LightComponent*> lights;
	std::vector<CameraComponent*> cameras;
	bool useMultiThreadedRendering=true;
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