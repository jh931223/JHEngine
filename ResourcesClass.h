#pragma once
class Mesh;
class TextureClass;
class Material;
class ShaderClass;
class RenderTextureClass;
#include <map>
#include <string>
#include"Singleton.h"
//using namespace std;
class ResourcesClass : public Singleton<ResourcesClass>
{


public:
	ResourcesClass();
	~ResourcesClass();
	void Initialize(HWND hwnd);
	static ResourcesClass* GetInstance();
	Material* FindMaterial(std::string _name);
	Mesh* FindMesh(std::string _name);
	TextureClass* FindTexture(std::string _name);
	ShaderClass* FindShader(std::string _name);
	RenderTextureClass* FindRenderTexture(std::string _name);
private:
	std::map<std::string, Material*> materialMap;
	std::map<std::string, TextureClass*> textureMap;
	std::map<std::string, Mesh*> meshMap;
	std::map<std::string, ShaderClass*> shaderMap;
	std::map<std::string, RenderTextureClass*> rttMap;

	void InitializeMesh(HWND hwnd);
	void InitializeTexture(HWND hwnd);
	void InitializeMaterial(HWND hwnd);
	void InitializeShader(HWND hwnd);
	void InitializeRenderTexture(HWND hwnd);
};

