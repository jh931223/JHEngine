cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};



cbuffer PSLightBufferType
{
	float4 ambientColor;
	float4 diffuseColor;
};

cbuffer VSLightBufferType
{
	float4 cameraPos;
	float3 lightDir;
	float padding;
};
cbuffer VSLightMatrixBuffer
{
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};


struct vInput
{
	float4 position : POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
};

struct v2f
{
	float4 position : SV_POSITION;
	float3 worldNormal : NORMAL;
	float2 uv : TEXCOORD0;
	float4 worldPos : TEXCOORD1;
	float3 viewDir : TEXCOORD2;
	float3 lightDir : TEXCOORD3;
	float4 lightClipPosition : TEXCOORD4;
};

Texture2D shaderTexture1 : register(t0);
Texture2D shaderNormalTex1 : register(t1);
Texture2D shaderTexture2 : register(t2);
Texture2D shaderNormalTex2 : register(t3);
Texture2D shaderTexture3 : register(t4);
Texture2D shaderNormalTex3 : register(t5);
Texture2D shadowMap : register(t6);


SamplerState SampleType: register(s0);
SamplerState SampleTypeClamp: register(s1);


v2f main(vInput input)
{
	v2f output;


	input.position.w = 1.0f;
	float4 worldPos = mul(input.position, worldMatrix);
	output.worldPos = worldPos;
	float3 viewDir = output.worldPos.xyz - cameraPos.xyz;
	viewDir = normalize(viewDir);
	output.viewDir = viewDir;

	output.position = mul(worldPos, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.uv = input.uv;

	float3 worldNormal = mul(normalize(input.normal), worldMatrix);
	output.worldNormal = worldNormal;

	float3 lDir = normalize(lightDir);
	output.lightDir = lDir;

	output.lightClipPosition = mul(worldPos, lightViewMatrix);
	output.lightClipPosition = mul(output.lightClipPosition, lightProjectionMatrix);

	return output;
}

float4 ps(v2f input) : SV_TARGET
{

	float3 blend_weights = abs(input.worldNormal.xyz);
	float tex_sharpness = 2.0f;
	blend_weights = (blend_weights - 0.2) * tex_sharpness;
	blend_weights = max(blend_weights, 0);
	blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z).xxx;

	float4 blended_color;
	float3 blended_bump_vec;
	{
		float tex_scale = 0.3f;
		float2 coord1 = input.worldPos.yz * tex_scale;
		float2 coord2 = input.worldPos.zx * tex_scale;
		float2 coord3 = input.worldPos.xy * tex_scale;
		float4 col1 = shaderTexture1.Sample(SampleType,coord1);
		float4 col2 = shaderTexture2.Sample(SampleType, coord2);
		float4 col3 = shaderTexture3.Sample(SampleType, coord3);

		float2 bumpFetch1 = shaderNormalTex1.Sample(SampleType, coord1).xy - 0.5;
		float2 bumpFetch2 = shaderNormalTex2.Sample(SampleType, coord2).xy - 0.5;
		float2 bumpFetch3 = shaderNormalTex3.Sample(SampleType, coord3).xy - 0.5;

		float3 bump1 = float3(0, bumpFetch1.x, bumpFetch1.y);
		float3 bump2 = float3(bumpFetch2.y, 0, bumpFetch2.x);
		float3 bump3 = float3(bumpFetch3.x, bumpFetch3.y, 0);

		blended_color = col1.xyzw * blend_weights.xxxx +
			col2.xyzw * blend_weights.yyyy +
			col3.xyzw * blend_weights.zzzz;
		blended_bump_vec = bump1.xyz * blend_weights.xxx +
			bump2.xyz * blend_weights.yyy +
			bump3.xyz * blend_weights.zzz;
	}
	float3 N_for_lighting = normalize(input.worldNormal + blended_bump_vec);

	
	float diffuse = dot(N_for_lighting, -input.lightDir);
	float3 reflection = reflect(input.lightDir, N_for_lighting);

	diffuse = saturate(diffuse);
	reflection = normalize(reflection);

	float3 viewDir = normalize(input.viewDir);
	float3 specular = 0;

	if (diffuse > 0)
	{
		specular = saturate(dot(reflection, -viewDir));
		specular = pow(specular, 1.3f);
	}

	float lightDepth = input.lightClipPosition.z / input.lightClipPosition.w;
	float2 shadowUV = input.lightClipPosition.xy / input.lightClipPosition.w;
	shadowUV.y = -shadowUV.y;
	shadowUV = shadowUV * 0.5f + 0.5f;

	float shadowMapDepth = shadowMap.Sample(SampleTypeClamp, shadowUV).r;
	lightDepth -= 0.0025f;
	if (lightDepth > shadowMapDepth)
	{
		diffuse *= 0.3f;
	}


	float3 d = blended_color * diffuse*diffuseColor;
	
	float3 ambient = ambientColor;

	float specularPower = 0.2f;
	return float4(d+ ambient +specular* specularPower,1);
}

