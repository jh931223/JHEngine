#pragma once
#include"SystemClass.h"
#include"GraphicsClass.h"
#include"D3DClass.h"
class ComputeShader : public AlignedAllocationPolicy<16>
{
public:
	ComputeShader();
	~ComputeShader();
	bool Initialize(ID3D11Device* device)
	{
		char shader[] =
			"int t;"  // implicit constant buffer
			"RWTexture2D<float4> y;"
			"[numthreads(16,16,1)]"
			"void cs_5_0(uint3 i:sv_dispatchthreadid)"
			"{"
			"float3 v=i/float3(640,400,1)-1,"
			"w=normalize(float3(v.x,-v.y*.8-1,2)),"
			"p=float3(sin(t*.0044),sin(t*.0024)+2,sin(t*.0034)-5);"
			"float b=dot(w,p),d=b*b-dot(p,p)+1,x=0;"
			"if(d>0){"
			"p-=w*(b+sqrt(d));"
			"x=pow(d,8);"
			"w=reflect(w,p);"
			"}"
			"if(w.y<0){"
			"p-=w*(p.y+1)/w.y;"
			"if(sin(p.z*6)*sin(p.x*6)>0)x+=2/length(p);"
			"}"
			"y[i.xy]=(abs(v.y-v.x)>.1&&abs(v.y+v.x)>.1)?x:float4(1,0,0,0);"
			"}";
		ID3D10Blob* pBlob;
		D3DCompile(shader, 1024, 0, 0, 0, "cs_5_0", "cs_5_0", 0, 0, &pBlob, 0);
		device->CreateComputeShader((void*)(((int*)pBlob)[3]), ((int*)pBlob)[2], NULL, &computeShader);
	}

	bool Dispatch(int x, int y, int z)
	{
		ID3D11DeviceContext* pD3D = SystemClass::GetInstance()->GetGraphics()->GetD3D()->GetDeviceContext();
		pD3D->CSSetShader(computeShader,NULL,0);
			
	}

	HRESULT CreateStructuredBufferOnGPU(ID3D11Device* pDevice,
		UINT uElementSize, UINT uCount, VOID* pInitData,
		ID3D11Buffer** ppBufOut)
	{

		*ppBufOut = NULL;
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = uElementSize * uCount;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = uElementSize;

		if (pInitData)
		{
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = pInitData;
			return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
		}
		else
			return pDevice->CreateBuffer(&desc, NULL, ppBufOut);
	}

	//for input buffer
	HRESULT CreateBufferShaderResourceView(ID3D11Device* pDevice, ID3D11Buffer* pBuffer,
		ID3D11ShaderResourceView** ppSRVOut)
	{

		D3D11_BUFFER_DESC descBuf;
		ZeroMemory(&descBuf, sizeof(descBuf));
		pBuffer->GetDesc(&descBuf);
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.BufferEx.FirstElement = 0;

		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
		{
			// This is a Raw Buffer
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
		}
		else
			if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
			{

				// This is a Structured Buffer
				desc.Format = DXGI_FORMAT_UNKNOWN;
				desc.BufferEx.NumElements =
					descBuf.ByteWidth / descBuf.StructureByteStride;
			}
			else
			{
				return E_INVALIDARG;
			}
		return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
	}

	//for output buffer    
	HRESULT CreateBufferUnorderedAccessView(ID3D11Device* pDevice, ID3D11Buffer* pBuffer,
		ID3D11UnorderedAccessView** ppUAVOut)
	{
		D3D11_BUFFER_DESC descBuf;
		ZeroMemory(&descBuf, sizeof(descBuf));
		pBuffer->GetDesc(&descBuf);

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;

		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
		{
			// This is a Raw Buffer
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			// Format must be DXGI_FORMAT_R32_TYPELESS,
			// when creating Raw Unordered Access View

			desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			desc.Buffer.NumElements = descBuf.ByteWidth / 4;
		}
		else
			if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
			{
				// This is a Structured Buffer
				desc.Format = DXGI_FORMAT_UNKNOWN;
				// Format must be must be DXGI_FORMAT_UNKNOWN,
				// when creating a View of a Structured Buffer

				desc.Buffer.NumElements =
					descBuf.ByteWidth / descBuf.StructureByteStride;
			}
			else
			{
				return E_INVALIDARG;
			}
		return pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut);
	}

protected:
	ID3D11ComputeShader * computeShader;
};

