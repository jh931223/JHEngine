#include "stdafx.h"
#include "MarchingCubeCompute.h"


MarchingCubeCompute::MarchingCubeCompute()
{
}


MarchingCubeCompute::~MarchingCubeCompute()
{
	if (constantBuffer)
	{
		constantBuffer->Release();
		constantBuffer = 0;
	}
}

bool MarchingCubeCompute::Initialize(ID3D11Device * device)
{
	return CompileShader(device,L"hlsl/marchingCube.cs");
}

bool MarchingCubeCompute::BindConstantBuffers(ID3D11DeviceContext * dc, ShaderParameterCollections * params)
{
	//if(dc->Map(constantBuffer,0, D3D11_MAP_WRITE_DISCARD,0, )
	return false;
}

bool MarchingCubeCompute::BindShaderViews(ID3D11DeviceContext * dc, ShaderParameterCollections * params)
{
	dc->CSSetUnorderedAccessViews(0, 1, params->GetStructuredBuffer("vertices")->GetUAV(), 0);
	dc->CSSetUnorderedAccessViews(1, 1, params->GetStructuredBuffer("indices")->GetUAV(), 0);
	return false;
}

bool MarchingCubeCompute::GenerateBuffer(ID3D11DeviceContext * dc)
{
	ID3D11Device* device;
	dc->GetDevice(&device);
	HRESULT result;

	/// constant buffer 
	
	D3D11_BUFFER_DESC cBufferDesc;
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.StructureByteStride = 0;
	cBufferDesc.ByteWidth = sizeof(int);
	cBufferDesc.MiscFlags = 0;
	result = device->CreateBuffer(&cBufferDesc, NULL, &constantBuffer);
	if (FAILED(result))
	{
		return false;
	}


	return true;
}
