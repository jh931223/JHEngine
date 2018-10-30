#pragma once
#include<string>
#include"ShaderClass.h"
class ShaderClass;
class ShaderParameterCollections;

enum QueueState
{
	Geometry, Transparent = 3000, AlphaTest = 3500
};
class Material
{

public:
	Material();
	Material(const Material& mat);
	~Material();
	bool Render(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX);
	void SetShader(ShaderClass *_shader);
	ShaderClass* GetShader()
	{
		return shader;
	}
	ShaderParameterCollections* GetParams();
public:
	int Queue= QueueState::Geometry;
private:
	ShaderClass * shader;
	ShaderParameterCollections params;
};
