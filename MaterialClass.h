#pragma once
#include<vector>
class TextureClass;
class ShaderClass;

class MaterialClass
{

public:
	MaterialClass();
	~MaterialClass();
	bool Render(ID3D11DeviceContext*, const int, XMMATRIX, XMMATRIX, XMMATRIX);
	void SetTextureArray(int _index, TextureClass* _texture);
	void AddTexture(TextureClass* _texture);
	std::vector<TextureClass*>* GetTextureArray();
	void SetShader(ShaderClass *_shader);
private:
	ShaderClass * shader;
	std::vector<TextureClass*> textureArray;
};

