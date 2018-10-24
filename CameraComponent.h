#pragma once
#include<list>

class RenderTextureClass;
class Material;
class CameraComponent : public Component
{
public:
	CameraComponent();
	~CameraComponent();

	void ChangeDepth(int _depth);

	void Render();
	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);
	void GetViewMtrixForCanvas(XMMATRIX & viewMatrix);
	void SetProjectionParameters(float FOV, float aspectRatio, float nearPlane, float farPlane);
	RenderTextureClass* GetRenderTexture();
	void SetRenderTexture(RenderTextureClass* _bitmap);
	static CameraComponent* mainCamera();
private:
	XMMATRIX m_viewMatrix;
	XMMATRIX m_viewMatrixForCanvas;
	RenderTextureClass* renderTexture;
public:
	static std::list<CameraComponent*> cameras;
	enum BackGroundType
	{
		SolidColor,
		SkySphere
	};
	BackGroundType background;
	Material* skyMaterial;
	XMFLOAT4 backGroundColor=XMFLOAT4(0.4f,0.4f,0.8f,1);
	float m_fieldOfView = 0;
	float m_aspectRatio = 0;
	float m_nearPlane = 0;
	float m_farPlane = 0;
	int depth=0;
};