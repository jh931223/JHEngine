#pragma once

class CameraComponent : public Component
{
public:
	CameraComponent();
	virtual ~CameraComponent();

	void Render();
	void GetViewMatrix(XMMATRIX&);
	static CameraComponent* mainCamera();
private:
	XMMATRIX m_viewMatrix;
};