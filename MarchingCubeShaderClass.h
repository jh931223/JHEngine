#pragma once
#include "ShaderClass.h"
class MarchingCubeShaderClass : public ShaderClass
{
protected:
	struct MCBufferType
	{
		float isoLevel;
		float length;
		XMFLOAT2 unitSize;
	};
public:
	MarchingCubeShaderClass();
	MarchingCubeShaderClass(const MarchingCubeShaderClass&);
	~MarchingCubeShaderClass();

	bool Initialize(ID3D11Device*, HWND) override;

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFileName = NULL)override;
	void ShutdownShaderCustomBuffer()override;

	bool DrawCall(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, PARAM& params)override;
	void RenderShader(ID3D11DeviceContext*, int)override;
private:
	ID3D11Buffer* m_marchingInfoBuffer=nullptr;
	int vertCount;

};
