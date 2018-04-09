#pragma once
#include<vector>
#include<map>
#include<string>
class TextureClass;
class ShaderClass;

class MaterialClass
{

public:
	MaterialClass();
	~MaterialClass();
	bool Render(ID3D11DeviceContext*, const int, XMMATRIX, XMMATRIX, XMMATRIX);
	void SetShader(ShaderClass *_shader);
	template<class T> bool SetParameter(const std::string& key, T v);
private:
	ShaderClass * shader;
	std::map<std::string,void*> parameterArray;
};

template<class T>
inline bool MaterialClass::SetParameter(const std::string & key, T v)
{
	parameterArray[key] = v;
	return true;
}
