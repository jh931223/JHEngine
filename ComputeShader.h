#pragma once
#include"SystemClass.h"
#include"GraphicsClass.h"
#include"D3DClass.h"
#include<map>
#include"ShaderParameterCollections.h"
class ComputeShader : public AlignedAllocationPolicy<16>
{
public:
	ComputeShader() {}
	virtual ~ComputeShader()
	{
		if (computeShader)
		{
			computeShader->Release();
			computeShader = 0;
		}
	}
	virtual bool Initialize(ID3D11Device* device) = 0;
	bool Dispatch(ID3D11DeviceContext* deviceContext,int x, int y, int z, ShaderParameterCollections* params)
	{
		BindConstantBuffers(deviceContext, params);
		BindShaderViews(deviceContext, params);
		deviceContext->CSSetShader(computeShader, NULL, 0);
		deviceContext->Dispatch(x, y, z);
	}
protected:
	bool CompileShader(ID3D11Device * device, const WCHAR * csFilename)
	{
		ID3D10Blob* pBlob;
		HRESULT result;
		ID3D10Blob* errorMessage = nullptr;
		result = D3DCompileFromFile(csFilename, NULL, NULL, "main", "cs_5_0", 0, 0, &pBlob, 0);
		if (FAILED(result))
		{
			// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
			if (errorMessage)
			{
				printf("%s Error Fail",csFilename);
			}
			// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
			else
			{
				printf("%s Missing Shader File",csFilename);
			}
			return false;
		}

		result = device->CreateComputeShader((void*)(((int*)pBlob)[3]), ((int*)pBlob)[2], NULL, &computeShader);
		if (FAILED(result))
		{
			return false;
		}

		pBlob->Release();
		pBlob	=	0;

		ID3D11DeviceContext* dc;
		device->GetImmediateContext(&dc);
		return GenerateBuffer(dc);
	}
	virtual bool GenerateBuffer(ID3D11DeviceContext* dc) = 0;
	virtual bool BindConstantBuffers(ID3D11DeviceContext* dc, ShaderParameterCollections* params) = 0;
	virtual bool BindShaderViews(ID3D11DeviceContext* dc, ShaderParameterCollections* params) = 0;





protected:
	ID3D11ComputeShader * computeShader;
};

