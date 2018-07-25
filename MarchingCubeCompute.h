#pragma once
#include "ComputeShader.h"
class MarchingCubeCompute :
	public ComputeShader
{
public:
	MarchingCubeCompute();
	virtual ~MarchingCubeCompute();
	// ComputeShader��(��) ���� ��ӵ�
	virtual bool Initialize(ID3D11Device * device) override;
private:
	virtual bool BindConstantBuffers(ID3D11DeviceContext * pD3D, ShaderParameterCollections * params) override;
	virtual bool BindShaderViews(ID3D11DeviceContext * pD3D, ShaderParameterCollections * params) override;

	// ComputeShader��(��) ���� ��ӵ�
	virtual bool GenerateBuffer(ID3D11DeviceContext * dc) override;
public:
private:
	ID3D11Buffer * constantBuffer;
};

