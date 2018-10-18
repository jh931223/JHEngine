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
	// ���� ���̴��� �ִ� ��� ��� ������ ����ü�� �ۼ��մϴ�.
	if (!CreateConstantBuffer<MatrixBufferType>(device, &m_matrixBuffer))
		return false;
	if (!CreateConstantBuffer<LightMatrixBufferType>(device, &lightMatrixBuffer))
		return false;
	return true;
}

void DepthMapShaderClass::ShutdownShaderCustomBuffer()
{
	if (lightMatrixBuffer)
		lightMatrixBuffer->Release();
	lightMatrixBuffer = 0;
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

	if (FAILED(deviceContext->Map(lightMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}
	LightMatrixBufferType* dataPtr2 = (LightMatrixBufferType*)mappedResource.pData;
	XMMATRIX lightView;
	LightComponent::mainLight()->GetViewMatrix(lightView);
	dataPtr2->lightView = XMMatrixTranspose(lightView);
	XMMATRIX lightProj;
	LightComponent::mainLight()->GetProjectionMatrix(lightProj);
	dataPtr2->lightProjection = XMMatrixTranspose(lightProj);
	deviceContext->Unmap(lightMatrixBuffer, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	deviceContext->VSSetConstantBuffers(1, 1, &lightMatrixBuffer);
	return true;
}

void DepthMapShaderClass::RenderShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	// ���� �Է� ���̾ƿ��� �����մϴ�.
	deviceContext->IASetInputLayout(m_layout);

	// �ﰢ���� �׸� ���� ���̴��� �ȼ� ���̴��� �����մϴ�.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// �ȼ� ���̴����� ���÷� ���¸� �����մϴ�.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// �ﰢ���� �׸��ϴ�.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}