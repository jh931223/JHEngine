#include "stdafx.h"
#include "DepthMapShaderClass.h"
#include "LightComponent.h"


DepthMapShaderClass::DepthMapShaderClass()
{
}


DepthMapShaderClass::~DepthMapShaderClass()
{
}

bool DepthMapShaderClass::Initialize(ID3D11Device * device, HWND hwnd)
{
	return BuildShader(device,hwnd, L"hlsl/depthMap.hlsl", L"hlsl/depthMap.hlsl", NULL);
}

bool DepthMapShaderClass::BuildShader(ID3D11Device * device, HWND hwnd, const WCHAR * vsFilename, const WCHAR * psFilename, const WCHAR * gsFileName )
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[]
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0 }
	};
	if (!CreateVertexShader(device, hwnd, vsFilename, "vs_4_0", "main", polygonLayout, ARRAYSIZE(polygonLayout)))
		return false;
	if (!CreatePixelShader(device, hwnd, vsFilename, "ps_4_0", "ps"))
		return false;
	// 정점 셰이더에 있는 행렬 상수 버퍼의 구조체를 작성합니다.
	if (!CreateConstantBuffer<MatrixBufferType>(device, &m_matrixBuffer))
		return false;

	return true;
}

void DepthMapShaderClass::ShutdownShaderCustomBuffer()
{

}

bool DepthMapShaderClass::DrawCall(ID3D11DeviceContext * deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, PARAM & params)
{
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



	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	return true;
}

void DepthMapShaderClass::RenderShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	// 정점 입력 레이아웃을 설정합니다.
	deviceContext->IASetInputLayout(m_layout);

	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 픽셀 쉐이더에서 샘플러 상태를 설정합니다.
	SetSampler(deviceContext);

	// 삼각형을 그립니다.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
