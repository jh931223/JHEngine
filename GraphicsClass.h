#pragma once
#include<list>
/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 500.0f;
const float SCREEN_NEAR = 1.0f;
const int SHADOWMAP_WIDTH = 1024;
const int SHADOWMAP_HEIGHT = 1024;


class D3DClass;
class CameraComponent;
class LightComponent;
class ProjectionShaderClass;
class TextureClass;
class ViewPointClass;
class MeshRenderer;
class BitmapClass;
class TextureShaderClass;
class MaterialClass;
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
private:
	bool Render();
	bool RenderScene(CameraComponent* camera,MaterialClass* customMaterial);
	bool RenderCanvas(CameraComponent* camera);
	bool RenderToTexture(CameraComponent* camera);
	bool RenderToDepthTexture(CameraComponent* camera);

public:

	std::list<MeshRenderer*> meshRenderers;
	std::list<LightComponent*> lights;
	std::list<CameraComponent*> cameras;
private:
	D3DClass* m_Direct3D = nullptr;
	ProjectionShaderClass* m_ProjectionShader = nullptr;
	TextureClass* m_ProjectionTexture = nullptr;
	TextureShaderClass* textureShader = nullptr;
	ViewPointClass* m_ViewPoint = nullptr;
	BitmapClass* m_Bitmap = nullptr;
};