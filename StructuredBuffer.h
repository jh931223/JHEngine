#pragma once
class StructuredBuffer
{
public:
	enum STRUCTURED_BUFFER_TYPE
	{
		S_BUFFER_TYPE_SRV=1,
		S_BUFFER_TYPE_UAV=2
	};
private:
	ID3D11Buffer* buffer;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
public:
	StructuredBuffer();
	StructuredBuffer(ID3D11Device* pDevice, size_t stuctSize, UINT elementNum, VOID* data, BYTE _bufferType);
	~StructuredBuffer();
	HRESULT InitializeBuffer(ID3D11Device* pDevice,size_t stuctSize,UINT elementNum, VOID* data, BYTE _bufferType);
	void Release();
	ID3D11ShaderResourceView*const* GetSRV();
	ID3D11UnorderedAccessView** GetUAV();
};

