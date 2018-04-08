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

	std::map<std::string, MaterialClass*> materialMap;

public:
	ResourcesClass();
	~ResourcesClass();
	void Initialize(HWND hwnd);
	static ResourcesClass* GetInstance();
private:
	void InitializeMesh(HWND hwnd);
	void InitializeTexture(HWND hwnd);
	void InitializeMaterial(HWND hwnd);
	void InitializeShader(HWND hwnd);
};

