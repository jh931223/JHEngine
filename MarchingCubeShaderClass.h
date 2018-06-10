#pragma once
#include "ShaderClass.h"
class MarchingCubeShaderClass : public ShaderClass
{
protected:
	struct MCBufferType
	{
		XMFLOAT3 startPosition;
		float length;
	};
public:
	MarchingCubeShaderClass();
	MarchingCubeShaderClass(const MarchingCubeShaderClass&);
	~MarchingCubeShaderClass();

	bool Initialize(ID3D11Device*, HWND) override;
	void Shutdown()override;
	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX)override;

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFileName = NULL)override;
	void ShutdownShader()override;

	bool DrawCall(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX)override;
	void RenderShader(ID3D11DeviceContext*, int)override;
private:
	ID3D11Buffer* m_marchingInfoBuffer=nullptr;
	int vertCount;

};