#pragma once

class CameraComponent : public Component
{
public:
	CameraComponent();
	~CameraComponent();

	void Render();
	void GetViewMatrix(XMMATRIX&);

private:
	XMMATRIX m_viewMatrix;
};