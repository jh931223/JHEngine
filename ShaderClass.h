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
		// �������� ����� ���̴� �Ű� ������ �����մϴ�.
		if (!DrawCall(deviceContext, worldMatrix, viewMatrix, projectionMatrix, params))
		{
			return false;
		}
		// ������ ���۸� ���̴��� �������Ѵ�.
		RenderShader(deviceContext, indexCount);
	}
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
	{
		// ���� �޽����� ���â�� ǥ���մϴ�.
		OutputDebugStringA(reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));

		// ���� �޼����� ��ȯ�մϴ�.
		errorMessage->Release();
		errorMessage = 0;

		// ������ ������ ������ �˾� �޼����� �˷��ݴϴ�.
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
		// ���̾ƿ��� �����մϴ�.
		if (m_layout)
		{
			m_layout->Release();
			m_layout = 0;
		}
		// �ȼ� ���̴��� �����մϴ�.
		if (m_pixelShader)
		{
			m_pixelShader->Release();
			m_pixelShader = 0;
		}
		// ���ؽ� ���̴��� �����մϴ�.
		if (m_vertexShader)
		{
			m_vertexShader->Release();
			m_vertexShader = 0;
		}
		// ������Ʈ�� ���̴��� �����մϴ�.
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
		// ���� �Է� ���̾ƿ��� ����ϴ�.
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

		// ��� ���� �����͸� ����� �� Ŭ�������� ���� ���̴� ��� ���ۿ� ������ �� �ְ� �մϴ�.
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
		// ���ؽ� ���̴� �ڵ带 �������Ѵ�.
		ID3D10Blob* vertexShaderBuffer = nullptr;
		result = D3DCompileFromFile(vsFilename, NULL, NULL, methodName, version, D3D10_SHADER_ENABLE_STRICTNESS,
			0, &vertexShaderBuffer, &errorMessage);
		if (FAILED(result))
		{
			// ���̴� ������ ���н� �����޽����� ����մϴ�.
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
			}
			// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
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
		// ���ؽ� ���̴� �ڵ带 �������Ѵ�.
		ID3D10Blob* pixelShaderBuffer = nullptr;
		result = D3DCompileFromFile(psFilename, NULL, NULL, methodName, version, D3D10_SHADER_ENABLE_STRICTNESS,
			0, &pixelShaderBuffer, &errorMessage);
		if (FAILED(result))
		{
			// ���̴� ������ ���н� �����޽����� ����մϴ�.
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
			}
			// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
			else
			{
				MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			}
			return false;
		}

		// ���ۿ��� �ȼ� ���̴��� �����մϴ�.
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
