#include "stdafx.h"
#include "TextureClass.h"
#include "meshclass.h"
#include<vector>
#include<algorithm>
#include<iterator>
#include <fstream>
using namespace std;


Mesh::Mesh()
{
}

Mesh::Mesh(ID3D11Device* device, const char* modelFilename)
{
	InitializeModel(device, modelFilename);
}

Mesh::Mesh(ID3D11Device * device, int _width, int _height)
{
	InitializeWindow(device, _width,_height);
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


bool Mesh::InitializeModel(ID3D11Device* device, const char* modelFilename)
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
bool Mesh::InitializeWindow(ID3D11Device* device, int windowWidth,int windowHeight)
{
	// 윈도우 왼쪽의 화면 좌표를 계산합니다.
	float left = (float)((windowWidth / 2) * -1);

	// 윈도우 오른쪽의 화면 좌표를 계산합니다.
	float right = left + (float)windowWidth;

	// 윈도우 상단의 화면 좌표를 계산합니다.
	float top = (float)(windowHeight / 2);

	// 윈도우 하단의 화면 좌표를 계산합니다.
	float bottom = top - (float)windowHeight;

	// 정점 배열의 정점 수를 설정합니다.
	m_vertexCount = 6;

	// 인덱스 배열의 인덱스 수를 설정합니다.
	m_indexCount = m_vertexCount;

	// 정점 배열을 만듭니다.
	VertexBuffer* vertices = new VertexBuffer[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// 인덱스 배열을 만듭니다.
	unsigned long* indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// 정점 배열에 데이터를로드합니다.
	// 첫 번째 삼각형.
	vertices[0].position = XMFLOAT3(left, top, 0.0f);  // 왼쪽 위
	vertices[0].uv = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // 오른쪽 아래
	vertices[1].uv = XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // 왼쪽 아래
	vertices[2].uv = XMFLOAT2(0.0f, 1.0f);

	// 두 번째 삼각형.
	vertices[3].position = XMFLOAT3(left, top, 0.0f);  // 왼쪽 위
	vertices[3].uv = XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = XMFLOAT3(right, top, 0.0f);  // 오른쪽 위
	vertices[4].uv = XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // 오른쪽 아래
	vertices[5].uv = XMFLOAT2(1.0f, 1.0f);

	// 데이터로 인덱스 배열을로드합니다.
	for (int i = 0; i<m_indexCount; i++)
	{
		indices[i] = i;
	}
	// 정점 및 인덱스 버퍼를 초기화합니다.
	if (!InitializeBuffers(device))
	{
		return false;
	}    
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
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

int Mesh::GetVertexCount()
{
	return m_vertexCount;
}

VertexBuffer * Mesh::GetVertices()
{
	return vertices;
}

unsigned long * Mesh::GetIndices()
{
	return indices;
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
	if (vertices == 0||m_vertexCount==0)
	{
		return false;
	}
	

	// 정적 정점 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexBuffer) * m_vertexCount;
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

	//vertices = 0;

	if (indices == 0||m_indexCount==0)
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

	//indices = 0;
	
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
	UINT stride = sizeof(VertexBuffer);
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
	vertices = new VertexBuffer[m_vertexCount];
	if (indices)
		delete[] indices;
	indices = new unsigned long[m_indexCount];
	for (int i = 0; i < m_vertexCount; i++)
	{
		fin >> vertices[i].position.x >> vertices[i].position.y >> vertices[i].position.z;
		fin >> vertices[i].uv.x >> vertices[i].uv.y;
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

bool Mesh::SetVertices(VertexBuffer* _vertices,int _size)
{
	if (_size == 0)
		return false;
	if (vertices)
		delete[] vertices;
	vertices = new VertexBuffer[_size];
	std::copy<VertexBuffer*>(_vertices, _vertices + _size,  stdext::checked_array_iterator<VertexBuffer*>(vertices, _size));
	//for(int i=0;i<_size;i++)
	//	vertices[i] = _vertices[i];
	m_vertexCount = _size;
	return true;
}


bool Mesh::SetIndices(unsigned long* _indices, int _size)
{
	if (_size == 0)
		return false;
	if (indices)
		delete[] indices;
	indices = new unsigned long[_size];
	std::copy< unsigned long*>(_indices, _indices + _size, stdext::checked_array_iterator<unsigned long*>(indices, _size));
	//for (int i = 0; i<_size; i++)
	//	indices[i] = _indices[i];
	//indices = _indices;
	m_indexCount = _size;
	return true;
}

bool Mesh::RecalculateNormals()
{
	if (m_indexCount == 0)
		return false;
	if (m_vertexCount == 0)
		return false;
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
