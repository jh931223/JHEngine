#pragma once
#include<map>
#include"StructuredBuffer.h"
class TextureClass;
class RenderTextureClass;
class ShaderParameterCollections
{
	std::map<std::string, XMFLOAT4> params_Float4;
	std::map<std::string, XMFLOAT3> params_Float3;
	std::map<std::string, XMFLOAT2> params_Float2;
	std::map<std::string, int> params_Int;
	std::map<std::string, float> params_Float;
	std::map<std::string, TextureClass*> params_Texture;
	std::map<std::string, RenderTextureClass*> params_RenderTexture;
	std::map<std::string, StructuredBuffer*> params_StructuredBuffer;
public:
	ShaderParameterCollections() {}
	~ShaderParameterCollections()
	{
		if (params_StructuredBuffer.size()>0)
		{
			for (auto i : params_StructuredBuffer)
			{
				delete i.second;
			}
		}
	}
	inline void SetFloat4(std::string name, XMFLOAT4 f4) { params_Float4[name] = f4; }
	inline void SetFloat3(std::string name, XMFLOAT3 f3) { params_Float3[name] = f3; }
	inline void SetFloat2(std::string name, XMFLOAT2 f2) { params_Float2[name] = f2; }
	inline void SetFloat(std::string name, float f) { params_Float[name] = f; }
	inline void SetInt(std::string name, int i) { params_Int[name] = i; }
	inline void SetTexture(std::string name, TextureClass* tC) { params_Texture[name] = tC; }
	inline void SetRenderTexture(std::string name, RenderTextureClass* tC) { params_RenderTexture[name] = tC; }
	inline void SetStructuredBuffer(std::string name, StructuredBuffer* srv) { if (params_StructuredBuffer[name]) { params_StructuredBuffer[name]->Release(); } params_StructuredBuffer[name] = srv; }
	inline XMFLOAT4 GetFloat4(std::string name) { return params_Float4[name]; }
	inline XMFLOAT3 GetFloat3(std::string name) { return params_Float3[name]; }
	inline XMFLOAT2 GetFloat2(std::string name) { return params_Float2[name]; }
	inline float GetFloat(std::string name) { return params_Float[name]; }
	inline int GetInt(std::string name) { return params_Int[name]; }
	inline TextureClass* GetTexture(std::string name) { return params_Texture[name]; }
	inline RenderTextureClass* GetRenderTexture(std::string name) { return params_RenderTexture[name]; }
	inline StructuredBuffer* GetStructuredBuffer(std::string name) {
		if (!params_StructuredBuffer[name]) { return NULL; } return params_StructuredBuffer[name];
	}
};
