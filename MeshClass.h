#pragma once
#include<vector>

struct VertexBuffer
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT4 color;
	void SetPosition(float x, float y, float z) { position = XMFLOAT3(x, y, z); }
};
class Mesh
{
public:


public:
	Mesh();
	Mesh(ID3D11Device*, const char*);
	Mesh(const Mesh&);
	~Mesh();

	bool Initialize(ID3D11Device*, const char*);
	bool InitializeBuffers(ID3D11Device*);
	void Shutdown();
	void ShutdownBuffers();
	void ReleaseModel();
	void Render(ID3D11DeviceContext*);
	bool SetVertices(VertexBuffer*, int);
	bool SetIndices(unsigned long*,int);
	bool RecalculateNormals();

	int GetIndexCount();
	VertexBuffer* vertices;
	ID3D11Buffer* const* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();

private:
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadModel(const char*);

private:
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	int m_vertexCount = 0;
	int m_indexCount = 0;
	unsigned long* indices;

};