#pragma once
#include <map>
#include <string>
class TextureClass;
class RenderTextureClass;
class ShaderParameterCollections
{
	std::map<std::string, XMFLOAT4> params_Float4;
	std::map<std::string, XMFLOAT3> params_Float3;
	std::map<std::string, int> params_Int;
	std::map<std::string, float> params_Float;
	std::map<std::string, TextureClass*> params_Texture;
	std::map<std::string, RenderTextureClass*> params_RenderTexture;
public:

	inline void SetFloat4(std::string name, XMFLOAT4 f4) { params_Float4[name] = f4; }
	inline void SetFloat3(std::string name, XMFLOAT3 f3) { params_Float3[name] = f3; }
	inline void SetFloat(std::string name, float f) { params_Float[name] = f; }
	inline void SetInt(std::string name, int i) { params_Int[name] = i; }
	inline void SetTexture(std::string name, TextureClass* tC) { params_Texture[name] = tC; }
	inline void SetRenderTexture(std::string name, RenderTextureClass* tC) { params_RenderTexture[name] = tC; }

	inline XMFLOAT4 GetFloat4(std::string name) { return params_Float4[name]; }
	inline XMFLOAT3 GetFloat3(std::string name) { return params_Float3[name]; }
	inline float GetFloat(std::string name) { return params_Float[name]; }
	inline int GetInt(std::string name) { return params_Int[name]; }
	inline TextureClass* GetTexture(std::string name) { return params_Texture[name]; }
	inline RenderTextureClass* GetRenderTexture(std::string name) { return params_RenderTexture[name]; }
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
	~ShaderClass() {};

	virtual bool Initialize(ID3D11Device*, HWND) = 0;
	virtual void Shutdown() = 0;
	virtual bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, PARAM*) = 0;
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

protected:
	virtual bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFileName=NULL) = 0;
	virtual void ShutdownShader() = 0;
	virtual bool DrawCall(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, PARAM*) = 0;
	virtual void RenderShader(ID3D11DeviceContext*, int)=0;
	bool CreateVertexLayout(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer)
	{
		HRESULT result;
		// ���̾ƿ��� ��� ���� �����ɴϴ�.
		UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		// ���� �Է� ���̾ƿ��� ����ϴ�.
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

	// �� ������ ModelClass�� ���̴��� VertexType ������ ��ġ�ؾ��մϴ�.
	const D3D11_INPUT_ELEMENT_DESC polygonLayout[3]
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
};
