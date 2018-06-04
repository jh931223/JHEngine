#pragma once

#define DIRECTINPUT_VERSION 0x0800

/////////////
// LINKING //
/////////////
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

//////////////
// INCLUDES //
//////////////
#include <d3d11_1.h>
#include <dinput.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "DDSTextureLoader.h"	// DDS 颇老 贸府
#include "WICTextureLoader.h"	// WIC 颇老 贸府
using namespace DirectX;

///////////////////////////
//  warning C4316 贸府侩  //
///////////////////////////
#include "AlignedAllocationPolicy.h"

template <class T> HRESULT CreateSRV(ID3D11Device* pDevice, int elementNum, ID3D11ShaderResourceView* srv,const T* buffer)
{
	HRESULT hr;
	ID3D11Buffer* ppBuf=NULL;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = sizeof(T) * elementNum;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(T);

	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = buffer;
	subResource.SysMemPitch = 0;
	subResource.SysMemSlicePitch = 0;

	hr=pDevice->CreateBuffer(&desc, &subResource, &ppBuf);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Format = DXGI_FORMAT_UNKNOWN;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	descSRV.Buffer.ElementWidth = elementNum;

	hr = pDevice->CreateShaderResourceView(ppBuf, &descSRV, &srv);
	return hr;
}
