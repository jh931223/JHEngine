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
	// ���� �� �ȼ� ���̴��� �ʱ�ȭ�մϴ�.
	return InitializeShader(device, hwnd, L"hlsl/marchingCube.hlsl", L"hlsl/marchingCube.hlsl", L"hlsl/marchingCube.hlsl");
}

bool MarchingCubeShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFileName)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;

	// ���ؽ� ���̴� �ڵ带 �������Ѵ�.
	ID3D10Blob* vertexShaderBuffer = nullptr;
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "vs", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// ���̴� ������ ���н� �����޽����� ����մϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// ������Ʈ�� ���̴� �ڵ带 �������Ѵ�.
	ID3D10Blob* geometryShaderBuffer = nullptr;
	result = D3DCompileFromFile(gsFileName, NULL, NULL, "gs", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &geometryShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// ���̴� ������ ���н� �����޽����� ����մϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, gsFileName);
		}
		// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
		else
		{
			MessageBox(hwnd, gsFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	// �ȼ� ���̴� �ڵ带 �������Ѵ�.
	ID3D10Blob* pixelShaderBuffer = nullptr;
	result = D3DCompileFromFile(psFilename, NULL, NULL, "ps", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// ���̴� ������ ���н� �����޽����� ����մϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	// ���۷κ��� ���� ���̴��� �����Ѵ�.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
		&m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// ���ۿ��� �ȼ� ���̴��� �����մϴ�.
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
	// ���ۿ��� ������Ʈ�� ���̴��� �����մϴ�.
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

	// �� �̻� ������ �ʴ� ���� ���̴� �۹��� �ȼ� ���̴� ���۸� �����մϴ�.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	geometryShaderBuffer->Release();
	geometryShaderBuffer = 0;
	// ���� ���̴��� �ִ� ��� ��� ������ ����ü�� �ۼ��մϴ�.

	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// ��� ���� �����͸� ����� �� Ŭ�������� ���� ���̴� ��� ���ۿ� ������ �� �ְ� �մϴ�.
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

	// �ؽ�ó ���÷� ���� ����ü�� ���� �� �����մϴ�.
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

	// �ؽ�ó ���÷� ���¸� ����ϴ�.
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
	// ��� ������ ������ �� �� �ֵ��� ��޴ϴ�.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	ID3D11Device* pDevice;
	deviceContext->GetDevice(&pDevice);
	// ��� ������ �����Ϳ� ���� �����͸� �����ɴϴ�.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// ����� transpose�Ͽ� ���̴����� ����� �� �ְ� �մϴ�
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);


	// ��� ���ۿ� ����� �����մϴ�.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// ��� ������ ����� Ǳ�ϴ�.
	deviceContext->Unmap(m_matrixBuffer, 0);




	// ���� ���̴������� ��� ������ ��ġ�� �����մϴ�.
	unsigned int bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
	deviceContext->GSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);


	// light constant buffer�� ��� �� �ֵ��� ����Ѵ�.
	if (FAILED(deviceContext->Map(m_marchingInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// ��� ������ �����Ϳ� ���� �����͸� �����ɴϴ�.
	MCBufferType* dataPtr2 = (MCBufferType*)mappedResource.pData;
	dataPtr2->isoLevel = params.GetFloat("isoLevel");
	dataPtr2->length = params.GetInt("length");
	dataPtr2->unitSize = params.GetFloat2("unitSize");
	deviceContext->Unmap(m_marchingInfoBuffer, 0);
	bufferNumber = 0;
	deviceContext->GSSetConstantBuffers(1, 1, &m_marchingInfoBuffer);
	deviceContext->GSSetShaderResources(0, 1, params.GetStructuredBuffer("chunksData")->GetSRV());

	// �ȼ� ���̴����� ���̴� �ؽ�ó ���ҽ��� �����մϴ�.
	deviceContext->PSSetShaderResources(0, 1, params.GetTexture("Texture1")->GetResourceView());
	deviceContext->PSSetShaderResources(1, 1, params.GetTexture("Texture2")->GetResourceView());
	deviceContext->PSSetShaderResources(2, 1, params.GetTexture("Texture3")->GetResourceView());
	vertCount = params.GetInt("vertCount");
	// ������Ʈ�� ���̴��� ���� ����
	return true;
}


void MarchingCubeShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// ���� �Է� ���̾ƿ��� �����մϴ�.
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	// �ﰢ���� �׸� ���� ���̴��� �ȼ� ���̴��� �����մϴ�.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->GSSetShader(m_geometryShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// �ȼ� ���̴����� ���÷� ���¸� �����մϴ�.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// �ﰢ���� �׸��ϴ�.
	deviceContext->Draw(vertCount, 0);
	//deviceContext->DrawIndexed(indexCount, 0, 0);
}