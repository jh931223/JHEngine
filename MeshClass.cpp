#include "stdafx.h"
#include "TextureClass.h"
#include "meshclass.h"
#include<vector>

#include <fstream>
using namespace std;


Mesh::Mesh()
{
}

Mesh::Mesh(ID3D11Device* device, const char* modelFilename)
{
	Initialize(device, modelFilename);
}


Mesh::Mesh(const Mesh& other)
{
}


Mesh::~Mesh()
{
	if (m_vertexBuffer)
	{
	}
}


bool Mesh::Initialize(ID3D11Device* device, const char* modelFilename)
{
	// �� �����͸� �ε��մϴ�.
	if (!LoadModel(modelFilename))
	{
		return false;
	}

	// ���� �� �ε��� ���۸� �ʱ�ȭ�մϴ�.
	if (!InitializeBuffers(device))
	{
		return false;
	}
	return true;
}


void Mesh::Shutdown()
{

	// ���ؽ� �� �ε��� ���۸� �����մϴ�.
	ShutdownBuffers();

	// �� ������ ��ȯ
	ReleaseModel();
}


void Mesh::Render(ID3D11DeviceContext* deviceContext)
{
	// �׸��⸦ �غ��ϱ� ���� �׷��� ������ ���ο� �������� �ε��� ���۸� �����ϴ�.
	RenderBuffers(deviceContext);
}


int Mesh::GetIndexCount()
{
	return m_indexCount;
}

ID3D11Buffer * const* Mesh::GetVertexBuffer()
{
	return &m_vertexBuffer;
}

ID3D11Buffer * Mesh::GetIndexBuffer()
{
	return m_indexBuffer;
}




bool Mesh::InitializeBuffers(ID3D11Device* device)
{
	if (vertices == 0)
	{
		return false;
	}
	if (indices == 0)
	{
		return false;
	}
	

	// ���� ���� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertxBuffer) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource ������ ���� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// ���� ���� ���۸� ����ϴ�.
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)))
	{
		return false;
	}

	// ���� �ε��� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// �ε��� �����͸� ����Ű�� ���� ���ҽ� ����ü�� �ۼ��մϴ�.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// �ε��� ���۸� �����մϴ�.
	if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)))
	{
		return false;
	}

	vertices = 0;
	indices = 0;
	
	return true;
}


void Mesh::ShutdownBuffers()
{
	// �ε��� ���۸� �����մϴ�.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// ���� ���۸� �����մϴ�.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}


void Mesh::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	// ���� ������ ������ �������� �����մϴ�.
	UINT stride = sizeof(VertxBuffer);
	UINT offset = 0;

	// ������ �� �� �ֵ��� �Է� ��������� ���� ���۸� Ȱ������ �����մϴ�.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// ������ �� �� �ֵ��� �Է� ��������� �ε��� ���۸� Ȱ������ �����մϴ�.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// ���� ���۷� �׸� �⺻���� �����մϴ�. ���⼭�� �ﰢ������ �����մϴ�.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


bool Mesh::LoadModel(const char * filename)
{
	// �� ������ ���ϴ�.
	ifstream fin;
	fin.open(filename);

	// ������ �� �� ������ �����մϴ�.
	if (fin.fail())
	{
		return false;
	}

	// ���ؽ� ī��Ʈ�� ������ �д´�.
	char input = 0;
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// ���ؽ� ī��Ʈ�� �д´�.
	fin >> m_vertexCount;

	// �ε����� ���� ���� ���� ���� �����մϴ�.
	m_indexCount = m_vertexCount;

	// �о� ���� ���� ������ ����Ͽ� ���� ����ϴ�.

	// �������� ���� �κб��� �д´�.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// ���ؽ� �����͸� �н��ϴ�.
	if (vertices)
		delete[] vertices;
	vertices = new VertxBuffer[m_vertexCount];
	if (indices)
		delete[] indices;
	indices = new unsigned long[m_indexCount];
	for (int i = 0; i < m_vertexCount; i++)
	{
		fin >> vertices[i].position.x >> vertices[i].position.y >> vertices[i].position.z;
		fin >> vertices[i].texture.x >> vertices[i].texture.y;
		fin >> vertices[i].normal.x >> vertices[i].normal.y >> vertices[i].normal.z;
		indices[i] = i;
	}

	// �� ������ �ݴ´�.
	fin.close();

	return true;
}


void Mesh::ReleaseModel()
{
	if (vertices)
	{
		delete[] vertices;
	}
	if (indices)
	{
		delete[] indices;
	}
}

bool Mesh::SetVertices(VertxBuffer* _vertices,int _size)
{
	vertices = _vertices;
	m_vertexCount = _size;
	return false;
}


bool Mesh::SetIndices(unsigned long* _indices, int _size)
{
	indices = _indices;
	m_indexCount = _size;
	return false;
}

bool Mesh::RecalculateNormals()
{
	if (vertices == 0)
		return false;
	if (indices == 0)
		return false;
	XMVECTOR V1, V2,UP;
	XMFLOAT3 n(0, 1, 0);
	XMFLOAT3 f1, f2;
	for (int i = 0; i < m_indexCount; i += 3)
	{
		f1 = vertices[indices[i]].position - vertices[indices[i+1]].position;
		f1=Normalize3(f1);
		f2 = vertices[indices[i+2]].position - vertices[indices[i+1]].position;
		f2 = Normalize3(f2);
		V1=XMLoadFloat3(&f1);
		V2=XMLoadFloat3(&f2);
		UP=XMVector3Cross(V1, V2);
		UP=XMVector3Normalize(UP);
		XMStoreFloat3(&n, UP);
		vertices[indices[i]].normal = n;
		vertices[indices[i+1]].normal = n;
		vertices[indices[i+2]].normal = n;
	}
	return true;
}
