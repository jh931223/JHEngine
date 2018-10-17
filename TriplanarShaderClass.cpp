#include "stdafx.h"
#include "TriplanarShaderClass.h"
#include"LightComponent.h"
#include"CameraComponent.h"

TriplanarShaderClass::TriplanarShaderClass()
{
}


TriplanarShaderClass::~TriplanarShaderClass()
{
}

bool TriplanarShaderClass::Initialize(ID3D11Device * device, HWND hwnd)
{
	return InitializeShader(device,hwnd, L"hlsl/triplanar.hlsl", L"hlsl/triplanar.hlsl",NULL);
}

bool TriplanarShaderClass::InitializeShader(ID3D11Device * device, HWND hwnd, const WCHAR * vsFilename, const WCHAR * psFilename, const WCHAR * gsFileName)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[]
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
	if (!CreateVertexShader(device, hwnd, vsFilename, "vs_5_0", "main", polygonLayout, ARRAYSIZE(polygonLayout)))
		return false;
	if(!CreatePixelShader(device, hwnd, vsFilename, "ps_5_0", "ps"))
		return false;
	// 정점 셰이더에 있는 행렬 상수 버퍼의 구조체를 작성합니다.
	if (!CreateConstantBuffer<MatrixBufferType>(device,&m_matrixBuffer))
		return false;
	if (!CreateConstantBuffer<VSLightBufferType>(device, &vsLightBuffer))
		return false;
	if (!CreateConstantBuffer<PSLightBufferType>(device, &psLightBuffer))
		return false;
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
	return true;
}

void TriplanarShaderClass::ShutdownShaderCustomBuffer()
{
	if (psLightBuffer)
	{
		psLightBuffer->Release();
	}
	psLightBuffer = 0;
	if (vsLightBuffer)
	{
		vsLightBuffer->Release();
	}
	vsLightBuffer = 0;
}

bool TriplanarShaderClass::DrawCall(ID3D11DeviceContext * deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, PARAM & params)
{
	// 상수 버퍼의 내용을 쓸 수 있도록 잠급니다.
	//printf("dc : %x\n", deviceContext);
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	if (FAILED(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;
	deviceContext->Unmap(m_matrixBuffer, 0);

	if (FAILED(deviceContext->Map(psLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}
	PSLightBufferType* dataPtr2 = (PSLightBufferType*)mappedResource.pData;
	dataPtr2->ambientColor = LightComponent::mainLight()->GetAmbientColor();
	dataPtr2->diffuseColor = LightComponent::mainLight()->GetDiffuseColor();
	deviceContext->Unmap(psLightBuffer, 0);


	if (FAILED(deviceContext->Map(vsLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}
	VSLightBufferType* dataPtr3 = (VSLightBufferType*)mappedResource.pData;
	dataPtr3->lightDir = LightComponent::mainLight()->transform()->forward();
	XMFLOAT3 camPos = ShaderClass::renderCam->transform()->GetWorldPosition();
	dataPtr3->cameraPos = XMFLOAT4(camPos.x,camPos.y,camPos.z,1);
	deviceContext->Unmap(vsLightBuffer, 0);


	// 마지막으로 정점 셰이더의 상수 버퍼를 바뀐 값으로 바꿉니다.
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	deviceContext->VSSetConstantBuffers(1, 1, &vsLightBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &psLightBuffer);

	// 픽셀 셰이더에서 셰이더 텍스처 리소스를 설정합니다.
	deviceContext->PSSetShaderResources(0, 1, params.GetTexture("Texture1")->GetResourceView());
	deviceContext->PSSetShaderResources(3, 1, params.GetTexture("Texture1Normal")->GetResourceView());
	deviceContext->PSSetShaderResources(1, 1, params.GetTexture("Texture2")->GetResourceView());
	deviceContext->PSSetShaderResources(4, 1, params.GetTexture("Texture2Normal")->GetResourceView());
	deviceContext->PSSetShaderResources(2, 1, params.GetTexture("Texture3")->GetResourceView());
	deviceContext->PSSetShaderResources(5, 1, params.GetTexture("Texture3Normal")->GetResourceView());
	return true;
}

void TriplanarShaderClass::RenderShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	// 정점 입력 레이아웃을 설정합니다.
	deviceContext->IASetInputLayout(m_layout);

	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 픽셀 쉐이더에서 샘플러 상태를 설정합니다.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// 삼각형을 그립니다.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
