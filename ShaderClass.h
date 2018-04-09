#pragma once
#include <map>
#include <string>
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
	virtual bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, void*) = 0;
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
	virtual bool InitializeShader(ID3D11Device*, HWND, const WCHAR*, const WCHAR*) = 0;
	virtual void ShutdownShader() = 0;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, void*) = 0;
	virtual void RenderShader(ID3D11DeviceContext*, int)=0;

	template<class T> T GetParam(std::map<std::string, void*>* p, const std::string& key)
	{
		return (T)(p->at(key));
	}
protected:
	ID3D11VertexShader * m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11Buffer* m_matrixBuffer = nullptr;

	ID3D11SamplerState* m_sampleState = nullptr;
};
