#pragma once
#include<list>

class CameraComponent : public Component
{
public:
	CameraComponent();
	virtual ~CameraComponent();

	void ChangeDepth(int _depth);

	void Render();
	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);
	static void GetDefaultViewMtrix(XMMATRIX & viewMatrix);
	void SetProjectionParameters(float FOV, float aspectRatio, float nearPlane, float farPlane);
	static CameraComponent* mainCamera();
private:
	XMMATRIX m_viewMatrix;
public:
	static std::list<CameraComponent*> cameras;
	float m_fieldOfView = 0;
	float m_aspectRatio = 0;
	float m_nearPlane = 0;
	float m_farPlane = 0;
	int depth=0;
};