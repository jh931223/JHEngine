#include "stdafx.h"
#include "StructuredBuffer.h"


StructuredBuffer::StructuredBuffer()
{
}

StructuredBuffer::StructuredBuffer(ID3D11Device * pDevice, size_t structSize, UINT elementNum, VOID * data, BYTE _bufferType)
{
	InitializeBuffer(pDevice, structSize, elementNum, data,_bufferType);
}


StructuredBuffer::~StructuredBuffer()
{
	Release();
}

HRESULT StructuredBuffer::InitializeBuffer(ID3D11Device* pDevice, size_t structSize, UINT elementNum, VOID* initData, BYTE _bufferType)
{
	HRESULT hr;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = structSize * elementNum;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = structSize;
	if (initData)
	{
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = initData;
		subResource.SysMemPitch = 0;
		subResource.SysMemSlicePitch = 0;
		hr = pDevice->CreateBuffer(&desc, &subResource, &buffer);
	}
	else
		hr = pDevice->CreateBuffer(&desc, NULL, &buffer);
	if (FAILED(hr))
	{
		return hr;
	}
	if (_bufferType&S_BUFFER_TYPE_SRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.Format = DXGI_FORMAT_UNKNOWN;
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		descSRV.BufferEx.FirstElement = 0;
		descSRV.BufferEx.NumElements = elementNum;

		hr = pDevice->CreateShaderResourceView(buffer, &descSRV, &srv);
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (_bufferType&S_BUFFER_TYPE_UAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
		ZeroMemory(&descUAV, sizeof(descUAV));
		descUAV.Format = DXGI_FORMAT_UNKNOWN;
		descUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		descUAV.Buffer.FirstElement = 0;
		descUAV.Buffer.NumElements = elementNum;

		hr = pDevice->CreateUnorderedAccessView(buffer, &descUAV, &uav);
		if (FAILED(hr))
		{
			return hr;
		}
	}
	return 1;
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

ID3D11UnorderedAccessView** StructuredBuffer::GetUAV()
{
	return &uav;
}
