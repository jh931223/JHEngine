#pragma once
#include<list>
class LightComponent : public Component
{
public:
	enum PROJECTION_TYPE
	{
		Perspective,
		Orthographic,
	};
public:
	LightComponent();
	~LightComponent();

	void SetAmbientColor(float, float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetPosition(float, float, float);
	void SetLookAt(float, float, float);

	XMFLOAT4 GetAmbientColor();
	XMFLOAT4 GetDiffuseColor();
	XMFLOAT3 GetPosition();

	void GenerateViewMatrix();
	void GeneratePerspectiveMatrix(float screenDepth, float screenNear);

	void GenerateOrthogrphicMatrix(float width, float depth, float nearPlane);

	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);
	static LightComponent* mainLight();
private:
	XMFLOAT4 m_ambientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 m_diffuseColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_lookAt = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;
public:
	static std::list<LightComponent*> lights;
};