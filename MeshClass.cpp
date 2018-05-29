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
	// 모델 데이터를 로드합니다.
	if (!LoadModel(modelFilename))
	{
		return false;
	}

	// 정점 및 인덱스 버퍼를 초기화합니다.
	if (!InitializeBuffers(device))
	{
		return false;
	}
	return true;
}


void Mesh::Shutdown()
{

	// 버텍스 및 인덱스 버퍼를 종료합니다.
	ShutdownBuffers();

	// 모델 데이터 반환
	ReleaseModel();
}


void Mesh::Render(ID3D11DeviceContext* deviceContext)
{
	// 그리기를 준비하기 위해 그래픽 파이프 라인에 꼭지점과 인덱스 버퍼를 놓습니다.
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
	

	// 정적 정점 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertxBuffer) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 만듭니다.
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)))
	{
		return false;
	}

	// 정적 인덱스 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
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
	// 인덱스 버퍼를 해제합니다.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// 정점 버퍼를 해제합니다.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}


void Mesh::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	// 정점 버퍼의 단위와 오프셋을 설정합니다.
	UINT stride = sizeof(VertxBuffer);
	UINT offset = 0;

	// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


bool Mesh::LoadModel(const char * filename)
{
	// 모델 파일을 엽니다.
	ifstream fin;
	fin.open(filename);

	// 파일을 열 수 없으면 종료합니다.
	if (fin.fail())
	{
		return false;
	}

	// 버텍스 카운트의 값까지 읽는다.
	char input = 0;
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// 버텍스 카운트를 읽는다.
	fin >> m_vertexCount;

	// 인덱스의 수를 정점 수와 같게 설정합니다.
	m_indexCount = m_vertexCount;

	// 읽어 들인 정점 개수를 사용하여 모델을 만듭니다.

	// 데이터의 시작 부분까지 읽는다.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// 버텍스 데이터를 읽습니다.
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

	// 모델 파일을 닫는다.
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
