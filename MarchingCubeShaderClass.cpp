#include "stdafx.h"
#include "MarchingCubeShaderClass.h"
#include "TextureClass.h"
#include <map>
#include <string>
#include "SystemClass.h"
#include"D3DClass.h"
MarchingCubeShaderClass::MarchingCubeShaderClass()
{
}


MarchingCubeShaderClass::MarchingCubeShaderClass(const MarchingCubeShaderClass& other)
{
}


MarchingCubeShaderClass::~MarchingCubeShaderClass()
{
}


bool MarchingCubeShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	// 정점 및 픽셀 쉐이더를 초기화합니다.
	return InitializeShader(device, hwnd, L"hlsl/marchingCube.hlsl", L"hlsl/marchingCube.hlsl", L"hlsl/marchingCube.hlsl");
}

bool MarchingCubeShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFileName)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;

	// 버텍스 쉐이더 코드를 컴파일한다.
	ID3D10Blob* vertexShaderBuffer = nullptr;
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "vs", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 지오메트리 쉐이더 코드를 컴파일한다.
	ID3D10Blob* geometryShaderBuffer = nullptr;
	result = D3DCompileFromFile(gsFileName, NULL, NULL, "gs", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &geometryShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, gsFileName);
		}
		// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
		else
		{
			MessageBox(hwnd, gsFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	// 픽셀 쉐이더 코드를 컴파일한다.
	ID3D10Blob* pixelShaderBuffer = nullptr;
	result = D3DCompileFromFile(psFilename, NULL, NULL, "ps", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	// 버퍼로부터 정점 셰이더를 생성한다.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
		&m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// 버퍼에서 픽셀 쉐이더를 생성합니다.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL,
		&m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	D3D11_SO_DECLARATION_ENTRY soDeclaration[] = {
		{ 0, "SV_POSITION", 0, 0, 4, 0 },
		{ 0, "NORMAL", 0, 0, 3, 0 },
		{ 0, "TEXCOORD", 0, 0, 2, 0 },
	};
	struct g2f
	{
		XMFLOAT4 position;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};
	UINT strides[] = { sizeof(g2f) };
	// 버퍼에서 지오메트리 쉐이더를 생성합니다.
	result=device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), NULL, &m_geometryShader);
	//result = device->CreateGeometryShaderWithStreamOutput(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(),soDeclaration, _countof(soDeclaration),strides,_countof(strides), 0, NULL,&m_geometryShader);
	if (FAILED(result))
	{
		return false;
	}


	D3D11_INPUT_ELEMENT_DESC polygonLayout[]
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT ,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};
	if (!CreateVertexLayout(device, vertexShaderBuffer, polygonLayout, ARRAYSIZE(polygonLayout)))
	{
		return false;
	}

	// 더 이상 사용되지 않는 정점 셰이더 퍼버와 픽셀 셰이더 버퍼를 해제합니다.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	geometryShaderBuffer->Release();
	geometryShaderBuffer = 0;
	// 정점 셰이더에 있는 행렬 상수 버퍼의 구조체를 작성합니다.

	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 합니다.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}


	D3D11_BUFFER_DESC mcInfoBufferDesc;
	ZeroMemory(&mcInfoBufferDesc, sizeof(mcInfoBufferDesc));
	mcInfoBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	mcInfoBufferDesc.ByteWidth = sizeof(MCBufferType);
	mcInfoBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mcInfoBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mcInfoBufferDesc.MiscFlags = 0;
	mcInfoBufferDesc.StructureByteStride = 0;
	result = device->CreateBuffer(&mcInfoBufferDesc, NULL, &m_marchingInfoBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 텍스처 샘플러 상태 구조체를 생성 및 설정합니다.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// 텍스처 샘플러 상태를 만듭니다.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}


	//int m_nBufferSize = 1000000;



	//D3D11_BUFFER_DESC bufferDesc ={

	//	m_nBufferSize,D3D11_USAGE_DEFAULT,D3D11_BIND_VERTEX_BUFFER|D3D11_BIND_STREAM_OUTPUT,0,0,0

	//};

	//device->CreateBuffer(&bufferDesc, NULL, &m_pBuffer);

	return true;
}


void MarchingCubeShaderClass::ShutdownShaderCustomBuffer()
{


	if (m_marchingInfoBuffer)
	{
		m_marchingInfoBuffer->Release();
		m_marchingInfoBuffer = 0;
	}
}



bool MarchingCubeShaderClass::DrawCall(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, PARAM& params)
{
	// 상수 버퍼의 내용을 쓸 수 있도록 잠급니다.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	ID3D11Device* pDevice;
	deviceContext->GetDevice(&pDevice);
	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// 행렬을 transpose하여 셰이더에서 사용할 수 있게 합니다
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);


	// 상수 버퍼에 행렬을 복사합니다.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// 상수 버퍼의 잠금을 풉니다.
	deviceContext->Unmap(m_matrixBuffer, 0);




	// 정점 셰이더에서의 상수 버퍼의 위치를 설정합니다.
	unsigned int bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
	deviceContext->GSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);


	// light constant buffer를 잠글 수 있도록 기록한다.
	if (FAILED(deviceContext->Map(m_marchingInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	MCBufferType* dataPtr2 = (MCBufferType*)mappedResource.pData;
	dataPtr2->isoLevel = params.GetFloat("isoLevel");
	dataPtr2->length = params.GetInt("length");
	dataPtr2->unitSize = params.GetFloat2("unitSize");
	deviceContext->Unmap(m_marchingInfoBuffer, 0);
	bufferNumber = 0;
	deviceContext->GSSetConstantBuffers(1, 1, &m_marchingInfoBuffer);
	deviceContext->GSSetShaderResources(0, 1, params.GetStructuredBuffer("chunksData")->GetSRV());

	// 픽셀 셰이더에서 셰이더 텍스처 리소스를 설정합니다.
	deviceContext->PSSetShaderResources(0, 1, params.GetTexture("Texture1")->GetResourceView());
	deviceContext->PSSetShaderResources(1, 1, params.GetTexture("Texture2")->GetResourceView());
	deviceContext->PSSetShaderResources(2, 1, params.GetTexture("Texture3")->GetResourceView());
	vertCount = params.GetInt("vertCount");
	// 지오메트리 셰이더에 값을 전달
	return true;
}


void MarchingCubeShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// 정점 입력 레이아웃을 설정합니다.
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->GSSetShader(m_geometryShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 픽셀 쉐이더에서 샘플러 상태를 설정합니다.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// 삼각형을 그립니다.
	deviceContext->Draw(vertCount, 0);
	//deviceContext->DrawIndexed(indexCount, 0, 0);
}