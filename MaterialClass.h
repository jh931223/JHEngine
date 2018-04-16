#pragma once
#include<string>
#include"ShaderClass.h"
class ShaderClass;
class ShaderParameterCollections;

class MaterialClass
{

public:
	MaterialClass();
	~MaterialClass();
	bool Render(ID3D11DeviceContext*, const int, XMMATRIX, XMMATRIX, XMMATRIX);
	void SetShader(ShaderClass *_shader);
	ShaderParameterCollections* GetParams();
private:
	ShaderClass * shader;
	ShaderParameterCollections params;
};
