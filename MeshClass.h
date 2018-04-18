#pragma once
#include<vector>

class Mesh
{
public:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};


public:
	Mesh();
	Mesh(ID3D11Device*, const char*);
	Mesh(const Mesh&);
	~Mesh();

	bool Initialize(ID3D11Device*, const char*);
	bool InitializeBuffers(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool SetVertices(VertexType*, int);
	bool SetIndices(unsigned long*,int);
	bool RecalculateNormals();

	int GetIndexCount();
	VertexType* vertices;


private:
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadModel(const char*);
	void ReleaseModel();

private:
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	int m_vertexCount = 0;
	int m_indexCount = 0;
	unsigned long* indices;

};