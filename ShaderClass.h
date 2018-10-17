#pragma once
#include <map>
#include <string>
#include"StructuredBuffer.h"
#include"TextureClass.h"
#include"ShaderParameterCollections.h"
#include"CameraComponent.h"
class TextureClass;
class RenderTextureClass;
class ID3D11ShaderResourceView;
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
	static void SetRenderCam(CameraComponent* _renderCam)
	{
		renderCam = _renderCam;
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
		if (m_matrixBuffer)
		{
			m_matrixBuffer->Release();
			m_matrixBuffer = 0;
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
	template <typename T> bool CreateConstantBuffer(ID3D11Device* device,ID3D11Buffer** _buffer)
	{
		D3D11_BUFFER_DESC matrixBufferDesc;
		matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof(T);
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		// 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 합니다.
		HRESULT result = device->CreateBuffer(&matrixBufferDesc, NULL, _buffer);
		if (FAILED(result))
		{
			return false;
		}
		return true;
	}
	bool CreateVertexShader(ID3D11Device * device, HWND hwnd,LPCWSTR vsFilename, LPCSTR version,LPCSTR methodName, D3D11_INPUT_ELEMENT_DESC polygonLayout[],int layoutSize)
	{
		HRESULT result;
		ID3D10Blob* errorMessage = nullptr;
		// 버텍스 쉐이더 코드를 컴파일한다.
		ID3D10Blob* vertexShaderBuffer = nullptr;
		result = D3DCompileFromFile(vsFilename, NULL, NULL, methodName, version, D3D10_SHADER_ENABLE_STRICTNESS,
			0, &vertexShaderBuffer, &errorMessage);
		if (FAILED(result))
		{
			// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
			}
			// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
			else
			{
				MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
			}
			return false;
		}
		result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
			&m_vertexShader);
		if (FAILED(result))
		{
			return false;
		}	

		if (!CreateVertexLayout(device, vertexShaderBuffer, polygonLayout, layoutSize))
		{
			return false;
		}

		vertexShaderBuffer->Release();
		vertexShaderBuffer = 0;
		return true;
	}
	bool CreatePixelShader(ID3D11Device * device, HWND hwnd, LPCWSTR psFilename, LPCSTR version, LPCSTR methodName)
	{
		HRESULT result;
		ID3D10Blob* errorMessage = nullptr;
		// 버텍스 쉐이더 코드를 컴파일한다.
		ID3D10Blob* pixelShaderBuffer = nullptr;
		result = D3DCompileFromFile(psFilename, NULL, NULL, methodName, version, D3D10_SHADER_ENABLE_STRICTNESS,
			0, &pixelShaderBuffer, &errorMessage);
		if (FAILED(result))
		{
			// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
			}
			// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
			else
			{
				MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			}
			return false;
		}

		// 버퍼에서 픽셀 쉐이더를 생성합니다.
		result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL,
			&m_pixelShader);
		if (FAILED(result))
		{
			return false;
		}
		pixelShaderBuffer->Release();
		pixelShaderBuffer = 0;
		return true;
	}
protected:
	ID3D11VertexShader * m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11GeometryShader* m_geometryShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	
	std::map<ID3D11DeviceContext*, ID3D11Buffer*> m_matrixBufferPerDC;// = nullptr;
	ID3D11Buffer* m_matrixBuffer = nullptr;
	ID3D11SamplerState* m_sampleState = nullptr;
	static CameraComponent* renderCam;
};
