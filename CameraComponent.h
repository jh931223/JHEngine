#pragma once

class CameraComponent : public Component
{
public:
	CameraComponent();
	virtual ~CameraComponent();

	void Render();
	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);
	static void GetDefaultViewMtrix(XMMATRIX & viewMatrix);
	void SetProjectionParameters(float FOV, float aspectRatio, float nearPlane, float farPlane);
	static CameraComponent* mainCamera();
private:
	XMMATRIX m_viewMatrix;
public:
	float m_fieldOfView = 0;
	float m_aspectRatio = 0;
	float m_nearPlane = 0;
	float m_farPlane = 0;
};