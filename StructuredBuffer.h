#pragma once
class StructuredBuffer
{
	ID3D11Buffer* buffer;
	ID3D11ShaderResourceView* srv;
public:
	StructuredBuffer();
	StructuredBuffer(ID3D11Device* pDevice, size_t stuctSize, int elementNum, void* data);
	~StructuredBuffer();
	HRESULT InitializeBuffer(ID3D11Device* pDevice,size_t stuctSize,int elementNum, void* data);
	void Release();
	ID3D11ShaderResourceView*const* GetSRV();

};

