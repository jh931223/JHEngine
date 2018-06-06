#include "stdafx.h"
#include "StructuredBuffer.h"


StructuredBuffer::StructuredBuffer()
{
}

StructuredBuffer::StructuredBuffer(ID3D11Device * pDevice, size_t structSize, int elementNum, void * data)
{
	InitializeBuffer(pDevice, structSize, elementNum, data);
}


StructuredBuffer::~StructuredBuffer()
{
	Release();
}

HRESULT StructuredBuffer::InitializeBuffer(ID3D11Device* pDevice, size_t structSize, int elementNum, void* data)
{
	HRESULT hr;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = structSize * elementNum;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = structSize;

	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = data;
	subResource.SysMemPitch = 0;
	subResource.SysMemSlicePitch = 0;

	hr = pDevice->CreateBuffer(&desc, &subResource, &buffer);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Format = DXGI_FORMAT_UNKNOWN;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	descSRV.Buffer.ElementWidth = elementNum;

	hr = pDevice->CreateShaderResourceView(buffer, &descSRV, &srv);
}

void StructuredBuffer::Release()
{
	if(srv)
		srv->Release();
	srv = 0;
	if(buffer)
		buffer->Release();
	buffer = 0;
}

ID3D11ShaderResourceView * const * StructuredBuffer::GetSRV()
{
	return &srv;
}
