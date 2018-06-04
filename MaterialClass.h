#pragma once
#include<string>
#include"ShaderClass.h"
class ShaderClass;
class ShaderParameterCollections;

class Material
{

public:
	Material();
	~Material();
	bool Render(ID3D11DeviceContext*, const int, XMMATRIX, XMMATRIX, XMMATRIX);
	void SetShader(ShaderClass *_shader,HWND hwnd);
	ShaderClass* GetShader()
	{
		return shader;
	}
	ShaderParameterCollections* GetParams();
private:
	ShaderClass * shader;
};
