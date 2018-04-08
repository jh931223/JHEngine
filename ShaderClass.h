#pragma once
#include <vector>
class TextureClass;
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
	virtual bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, std::vector<TextureClass*>*) = 0;
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
	virtual bool InitializeShader(ID3D11Device*, HWND, const WCHAR*, const WCHAR*) = 0;
	virtual void ShutdownShader() = 0;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, std::vector<TextureClass*>*) = 0;
	virtual void RenderShader(ID3D11DeviceContext*, int)=0;

protected:
	ID3D11VertexShader * m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11Buffer* m_matrixBuffer = nullptr;

	ID3D11SamplerState* m_sampleState = nullptr;
};
