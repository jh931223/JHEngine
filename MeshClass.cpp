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
	delete vertices;
	delete indices;
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
	if (uv == 0)
		return false;
	VertexType* vertType = new VertexType[m_vertexCount];
	for (int i = 0; i < m_vertexCount; i++)
	{
		vertType[i].position = vertices[i];
		vertType[i].normal = normals[i];
		vertType[i].texture = uv[i];
	}

	

	// ���� ���� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource ������ ���� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertType;
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

	delete[] vertType;
	vertType = 0;
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
	UINT stride = sizeof(VertexType);
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
	vertices = new XMFLOAT3[m_vertexCount];
	if (uv)
		delete[] uv;
	uv = new XMFLOAT2[m_vertexCount];
	if (normals)
		delete[] normals;
	normals = new XMFLOAT3[m_vertexCount];
	if (indices)
		delete[] indices;
	indices = new unsigned long[m_indexCount];
	for (int i = 0; i < m_vertexCount; i++)
	{
		fin >> vertices[i].x >> vertices[i].y >> vertices[i].z;
		fin >> uv[i].x >> uv[i].y;
		fin >> normals[i].x >> normals[i].y >> normals[i].z;
		indices[i] = i;
	}

	// �� ������ �ݴ´�.
	fin.close();

	return true;
}


void Mesh::ReleaseModel()
{
}

bool Mesh::SetVertices(XMFLOAT3* _vertices,int _size)
{
	if (vertices)
		delete[] vertices;
	vertices = new XMFLOAT3[_size];
	for (int i = 0; i < _size; i++)
		vertices[i] = _vertices[i];
	m_vertexCount = _size;
	return false;
}

bool Mesh::SetNormals(XMFLOAT3* _normals)
{
	if (normals)
		delete[] normals;
	normals = _normals;
	return false;
}
bool Mesh::SetUVs(XMFLOAT2* _uv)
{
	if (uv)
		delete[] uv;
	uv = new XMFLOAT2[m_vertexCount];
	for (int i = 0; i < m_vertexCount; i++)
		uv[i] = _uv[i];
	return false;
}

bool Mesh::SetIndices(unsigned long* _indices, int _size)
{
	if (indices)
		delete[] indices;
	indices = new DWORD[_size];
	for (int i = 0; i < _size; i++)
		indices[i] = _indices[i];
	m_indexCount = _size;
	return false;
}

bool Mesh::RecalculateNormals()
{
	if (vertices == 0)
		return false;
	if (indices == 0)
		return false;
	if (normals)
		delete[] normals;
	normals = new XMFLOAT3[m_vertexCount];
	XMVECTOR V1, V2,UP;
	XMFLOAT3 n(0, 1, 0);
	XMFLOAT3 f1, f2;
	for (int i = 0; i < m_indexCount; i += 3)
	{
		f1 = vertices[indices[i]] - vertices[indices[i+1]];
		f1=Normalize3(f1);
		f2 = vertices[indices[i+2]] - vertices[indices[i + 1]];
		f2 = Normalize3(f2);
		V1=XMLoadFloat3(&f1);
		V2=XMLoadFloat3(&f2);
		UP=XMVector3Cross(V1, V2);
		UP=XMVector3Normalize(UP);
		XMStoreFloat3(&n, UP);
		normals[indices[i]] = n;
		normals[indices[i+1]] = n;
		normals[indices[i+2]] = n;
	}
	return true;
}
