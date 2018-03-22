#pragma once
class Mesh;
class TextureClass;
class ResourcesClass
{
public:
	Mesh *m_GroundModel = nullptr;
	Mesh *m_CubeModel = nullptr;
public:
	ResourcesClass();
	~ResourcesClass();
	void Initialize();
	static ResourcesClass* GetInstance();
};

