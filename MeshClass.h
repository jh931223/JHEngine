#pragma once
#include<vector>
struct VertexBuffer
{
	XMFLOAT3 position=XMFLOAT3(0,0,0);
	XMFLOAT2 uv=XMFLOAT2(0,0);
	XMFLOAT3 normal=XMFLOAT3(0,0,0);
	XMFLOAT4 color=XMFLOAT4(0,0,0,0);
	void SetPosition(float x, float y, float z) { position = XMFLOAT3(x, y, z); }

};
class Mesh
{
public:


public:
	Mesh();
	Mesh(ID3D11Device*, const char*);
	Mesh(ID3D11Device*, int _width,int _height);
	Mesh(const Mesh&);
	~Mesh();

	bool InitializeModel(ID3D11Device*, const char*);
	bool InitializeWindow(ID3D11Device * device, int _width, int _height);
	bool InitializeBuffers(ID3D11Device*);
	void Shutdown();
	void ShutdownBuffers();
	void ReleaseModel();
	void Render(ID3D11DeviceContext*);
	bool SetVertices(VertexBuffer*, int);
	bool SetIndices(unsigned long*,int);
	bool RecalculateNormals();

	int GetIndexCount();
	int GetVertexCount();
	VertexBuffer* GetVertices();
	unsigned long* GetIndices();


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
	VertexBuffer* vertices;

};