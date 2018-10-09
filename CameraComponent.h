#pragma once
#include<list>

class RenderTextureClass;

class CameraComponent : public Component
{
public:
	CameraComponent();
	~CameraComponent();

	void ChangeDepth(int _depth);

	void Render();
	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);
	static void GetDefaultViewMtrix(XMMATRIX & viewMatrix);
	void SetProjectionParameters(float FOV, float aspectRatio, float nearPlane, float farPlane);
	RenderTextureClass* GetRenderTexture();
	void SetRenderTexture(RenderTextureClass* _bitmap);
	static CameraComponent* mainCamera();
private:
	XMMATRIX m_viewMatrix;
	RenderTextureClass* renderTexture;
public:
	static std::list<CameraComponent*> cameras;
	float m_fieldOfView = 0;
	float m_aspectRatio = 0;
	float m_nearPlane = 0;
	float m_farPlane = 0;
	int depth=0;
};