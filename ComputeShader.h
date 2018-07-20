#pragma once
#include"SystemClass.h"
#include"GraphicsClass.h"
#include"D3DClass.h"
#include<map>
#include"StructuredBuffer.h"
class CShaderParameterCollections
{
public:
	~CShaderParameterCollections()
	{
		if (params_SRV.size()>0)
		{
			for (auto i : params_SRV)
			{
				i.second->Release();
				delete i.second;
			}
		}
		if (params_UAV.size()>0)
		{
			for (auto i : params_UAV)
			{
				i.second->Release();
				delete i.second;
			}
		}
	}
private:
	std::map<std::string, StructuredBuffer*> params_SRV;
	std::map<std::string, StructuredBuffer*> params_UAV;
public:
	inline void SetSRV(std::string name, StructuredBuffer* srv) { if (params_SRV[name]) { params_SRV[name]->Release(); } params_SRV[name] = srv; }
	inline void SetUAV(std::string name, StructuredBuffer* uav) { if (params_UAV[name]) { params_UAV[name]->Release(); } params_UAV[name] = uav; }
	inline ID3D11ShaderResourceView*const* GetSRV(std::string name) {
		if (!params_SRV[name]) { return NULL; } return params_SRV[name]->GetSRV();
	}
	inline ID3D11UnorderedAccessView*const* GetUAV(std::string name) {
		if (!params_UAV[name]) { return NULL; } return params_UAV[name]->GetUAV();
	}
};
class ComputeShader : public AlignedAllocationPolicy<16>
{
public:
	ComputeShader();
	~ComputeShader();
	virtual bool Initialize(ID3D11Device* device, HWND hwnd) = 0;
	bool Dispatch(int x, int y, int z, CShaderParameterCollections* params)
	{
		ID3D11DeviceContext* pD3D = SystemClass::GetInstance()->GetGraphics()->GetD3D()->GetImmDeviceContext();

		BindConstantBuffers(pD3D, params);
		BindShaderResources(pD3D, params);
		
		pD3D->CSSetShader(computeShader, NULL, 0);
		pD3D->Dispatch(x, y, z);
	}
protected:
	bool InitializeShader(ID3D11Device * device, HWND hwnd, const WCHAR * csFilename)
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
				OutputShaderErrorMessage(errorMessage, hwnd, csFilename);
			}
			// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
			else
			{
				MessageBox(hwnd, csFilename, L"Missing Shader File", MB_OK);
			}
			return false;
		}

		result = device->CreateComputeShader((void*)(((int*)pBlob)[3]), ((int*)pBlob)[2], NULL, &computeShader);
		if (FAILED(result))
		{
			return false;
		}
		return true;
	}
	virtual bool BindConstantBuffers(ID3D11DeviceContext* pD3D, CShaderParameterCollections* params) = 0;
	virtual bool BindShaderResources(ID3D11DeviceContext* pD3D, CShaderParameterCollections* params) = 0;

	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
	{
		// 에러 메시지를 출력창에 표시합니다.
		OutputDebugStringA(reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));

		// 에러 메세지를 반환합니다.
		errorMessage->Release();
		errorMessage = 0;

		// 컴파일 에러가 있음을 팝업 메세지로 알려줍니다.
		MessageBox(hwnd, L"Error compiling shader.", shaderFilename, MB_OK);
	}




protected:
	ID3D11ComputeShader * computeShader;
};

