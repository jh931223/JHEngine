#include "stdafx.h"
#include "TextureShaderClass.h"
#include "RenderTextureClass.h"


TextureShaderClass::TextureShaderClass()
{
}


TextureShaderClass::~TextureShaderClass()
{
}

bool TextureShaderClass::Initialize(ID3D11Device * device, HWND hwnd)
{
	return BuildShader(device,hwnd, L"hlsl/TextureShader.hlsl", L"hlsl/TextureShader.hlsl",NULL);
}

bool TextureShaderClass::BuildShader(ID3D11Device * device, HWND hwnd, const WCHAR * vsFilename, const WCHAR * psFilename, const WCHAR * gsFileName)
{

	D3D11_INPUT_ELEMENT_DESC polygonLayout[]
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
	if (!CreateVertexShader(device, hwnd, vsFilename, "vs_4_0", "main", polygonLayout, ARRAYSIZE(polygonLayout)))
		return false;
	if (!CreatePixelShader(device, hwnd, psFilename, "ps_4_0", "ps"))
		return false;
	if (!CreateConstantBuffer<MatrixBufferType>(device, &m_matrixBuffer))
		return false;

	CreateSampler(device);
	return true;
}

void TextureShaderClass::ShutdownShaderCustomBuffer()
{
}

bool TextureShaderClass::DrawCall(ID3D11DeviceContext * deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, PARAM & params)
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
	auto tex = params.GetTexture("_MainTex");
	if(tex)
		deviceContext->PSSetShaderResources(0, 1, params.GetTexture("_MainTex")->GetResourceView());
	else
		deviceContext->PSSetShaderResources(0, 1, params.GetRenderTexture("_MainRenderTex")->GetResourceView());
	return true;
}

void TextureShaderClass::RenderShader(ID3D11DeviceContext * deviceContext, int indexCount)
{	// 정점 입력 레이아웃을 설정합니다.
	deviceContext->IASetInputLayout(m_layout);

	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 픽셀 쉐이더에서 샘플러 상태를 설정합니다.
	SetSampler(deviceContext);

	// 삼각형을 그립니다.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
