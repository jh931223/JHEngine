#pragma once


class Mesh
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	struct MeshType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	Mesh();
	Mesh(const Mesh&);
	~Mesh();

	bool Initialize(ID3D11Device*, const char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();


private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadModel(const char*);
	void ReleaseModel();

private:
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	int m_vertexCount = 0;
	int m_indexCount = 0;
	MeshType* m_mesh = nullptr;
};