#pragma once
class Mesh;
class TextureClass;
class MaterialClass;
class ShaderClass;
#include <map>
#include <string>
//using namespace std;
class ResourcesClass
{
public:
	Mesh *m_GroundModel = nullptr;
	Mesh *m_CubeModel = nullptr;
	TextureClass *t_cube = nullptr;
	TextureClass *t_Floor = nullptr;
	MaterialClass *mat_Cube = nullptr;
	ShaderClass* s_Texture = nullptr;


public:
	ResourcesClass();
	~ResourcesClass();
	void Initialize(HWND hwnd);
	static ResourcesClass* GetInstance();
	MaterialClass* FindMaterial(std::string _name);
	Mesh* FindMesh(std::string _name);
	TextureClass* FindTexture(std::string _name);
	ShaderClass* FindShader(std::string _name);
private:
	std::map<std::string, MaterialClass*> materialMap;
	std::map<std::string, TextureClass*> textureMap;
	std::map<std::string, Mesh*> meshMap;
	std::map<std::string, ShaderClass*> shaderMap;

	void InitializeMesh(HWND hwnd);
	void InitializeTexture(HWND hwnd);
	void InitializeMaterial(HWND hwnd);
	void InitializeShader(HWND hwnd);
};

