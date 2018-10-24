#pragma once
#include<string>
#include"ShaderClass.h"
class ShaderClass;
class ShaderParameterCollections;

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
private:
	ShaderClass * shader;
	ShaderParameterCollections params;
};
