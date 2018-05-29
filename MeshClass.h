#pragma once
#include<vector>

class Mesh
{
public:
	struct VertxBuffer
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		void SetPosition(float x, float y, float z) { position = XMFLOAT3(x, y, z); }
	};


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
	bool SetVertices(VertxBuffer*, int);
	bool SetIndices(unsigned long*,int);
	bool RecalculateNormals();

	int GetIndexCount();
	VertxBuffer* vertices;


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