#pragma once
#include <map>
#include <string>
#include"StructuredBuffer.h"
#include"TextureClass.h"
class TextureClass;
class RenderTextureClass;
class ID3D11ShaderResourceView;
class ShaderParameterCollections
{
	std::map<std::string, XMFLOAT4> params_Float4;
	std::map<std::string, XMFLOAT3> params_Float3;
	std::map<std::string, XMFLOAT2> params_Float2;
	std::map<std::string, int> params_Int;
	std::map<std::string, float> params_Float;
	std::map<std::string, TextureClass*> params_Texture;
	std::map<std::string, RenderTextureClass*> params_RenderTexture;
	std::map<std::string, StructuredBuffer*> params_StructuredBuffer;
public:
	ShaderParameterCollections() {}
	~ShaderParameterCollections()
	{
		if (params_StructuredBuffer.size()>0)
		{
			for (auto i : params_StructuredBuffer)
			{
				delete i.second;
			}
		}
	}
	inline void SetFloat4(std::string name, XMFLOAT4 f4) { params_Float4[name] = f4; }
	inline void SetFloat3(std::string name, XMFLOAT3 f3) { params_Float3[name] = f3; }
	inline void SetFloat2(std::string name, XMFLOAT2 f2) { params_Float2[name] = f2; }
	inline void SetFloat(std::string name, float f) { params_Float[name] = f; }
	inline void SetInt(std::string name, int i) { params_Int[name] = i; }
	inline void SetTexture(std::string name, TextureClass* tC) { params_Texture[name] = tC; }
	inline void SetRenderTexture(std::string name, RenderTextureClass* tC) { params_RenderTexture[name] = tC; }
	inline void SetSRV(std::string name, StructuredBuffer* srv) { if (params_StructuredBuffer[name]) { params_StructuredBuffer[name]->Release(); } params_StructuredBuffer[name] = srv; }
	inline XMFLOAT4 GetFloat4(std::string name) { return params_Float4[name]; }
	inline XMFLOAT3 GetFloat3(std::string name) { return params_Float3[name]; }
	inline XMFLOAT2 GetFloat2(std::string name) { return params_Float2[name]; }
	inline float GetFloat(std::string name) { return params_Float[name]; }
	inline int GetInt(std::string name) { return params_Int[name]; }
	inline TextureClass* GetTexture(std::string name) { return params_Texture[name]; }
	inline RenderTextureClass* GetRenderTexture(std::string name) { return params_RenderTexture[name]; }
	inline ID3D11ShaderResourceView*const* GetSRV(std::string name) {
		if (!params_StructuredBuffer[name]) { return NULL; } return params_StructuredBuffer[name]->GetSRV();
	}
};
typedef ShaderParameterCollections PARAM;
class ShaderClass : public AlignedAllocationPolicy<16>
{
protected:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	ShaderClass() {};
	ShaderClass(const ShaderClass&) {};
	virtual ~ShaderClass() {};

	virtual bool Initialize(ID3D11Device* device, HWND hwnd) = 0;
	void Shutdown()
	{
		ShutdownShader();
	}
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix,
		XMMATRIX viewMatrix, XMMATRIX projectionMatrix, PARAM& params)
	{
		// 렌더링에 사용할 셰이더 매개 변수를 설정합니다.
		if (!DrawCall(deviceContext, worldMatrix, viewMatrix, projectionMatrix, params))
		{
			return false;
		}
		// 설정된 버퍼를 셰이더로 렌더링한다.
		RenderShader(deviceContext, indexCount);
	}
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
	virtual bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFileName=NULL) = 0;
	virtual void ShutdownShaderCustomBuffer() = 0;
	void ShutdownShader() 
	{ 
		// 레이아웃을 해제합니다.
		if (m_layout)
		{
			m_layout->Release();
			m_layout = 0;
		}
		// 픽셀 쉐이더를 해제합니다.
		if (m_pixelShader)
		{
			m_pixelShader->Release();
			m_pixelShader = 0;
		}
		// 버텍스 쉐이더를 해제합니다.
		if (m_vertexShader)
		{
			m_vertexShader->Release();
			m_vertexShader = 0;
		}
		// 지오메트리 쉐이더를 해제합니다.
		if (m_geometryShader)
		{
			m_geometryShader->Release();
			m_geometryShader = 0;
		}
		if (m_sampleState)
		{
			m_sampleState->Release();
			m_sampleState = 0;
		}
		ShutdownShaderCustomBuffer();
	};
	virtual bool DrawCall(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix, PARAM& params) = 0;
	virtual void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount) =0;
	bool CreateVertexLayout(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer, const D3D11_INPUT_ELEMENT_DESC* polygonLayout, UINT numElements)
	{
		HRESULT result;
		// 정점 입력 레이아웃을 만듭니다.
		result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(), &m_layout);
		if (FAILED(result))
		{
			return false;
		}
		return true;
	}

protected:
	ID3D11VertexShader * m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11GeometryShader* m_geometryShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11Buffer* m_matrixBuffer = nullptr;
	ID3D11SamplerState* m_sampleState = nullptr;
};
