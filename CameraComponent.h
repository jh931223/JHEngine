#pragma once

class CameraComponent : public Component
{
public:
	CameraComponent();
	virtual ~CameraComponent();

	void Render();
	void GetViewMatrix(XMMATRIX&);
	static void GetDefaultViewMtrix(XMMATRIX & viewMatrix);
	static CameraComponent* mainCamera();
private:
	XMMATRIX m_viewMatrix;
};