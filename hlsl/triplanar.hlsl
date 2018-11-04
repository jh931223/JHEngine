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
	float shadowBias;
	float3 padding_PSLB;
};

cbuffer VSLightBufferType
{
	float4 cameraPos;
	float3 lightDir;
	float padding_vsLB;
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
	float4 color : COLOR;
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
	float4 color : COLOR;
};
Texture2D _Tex[8] :register(t0);
Texture2D _Normal[8] :register(t8);
Texture2D shadowMap : register(t20);


//Texture2D shaderTexture1 : register(t0);
//Texture2D shaderNormalTex1 : register(t1);
//Texture2D shaderTexture2 : register(t2);
//Texture2D shaderNormalTex2 : register(t3);
//Texture2D shaderTexture3 : register(t4);
//Texture2D shaderNormalTex3 : register(t5);
//Texture2D shadowMap : register(t6);


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

	output.color = normalize(input.color);

	return output;
}

float4 ps(v2f input) : SV_TARGET
{

	float3 blend_weights = abs(input.worldNormal.xyz);
	float tex_sharpness = 2.0f;
	blend_weights = (blend_weights - 0.2) * tex_sharpness;
	blend_weights = max(blend_weights, 0);
	blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z).xxx;
	//blend_weights /= dot(blend_weights, (float3)2);
	float4 blended_color;
	float3 blended_bump_vec;
	{
		float tex_scale = 0.1f;
		float2 coord1 = input.worldPos.yz * tex_scale;
		float2 coord2 = input.worldPos.zx * tex_scale;
		float2 coord3 = input.worldPos.xy * tex_scale;

		float4 col1[4];
		float4 col2[4];
		float4 col3[4];
		float2 bumpFetch1[4];
		float2 bumpFetch2[4];
		float2 bumpFetch3[4];
		float3 bump1[4];
		float3 bump2[4];
		float3 bump3[4];

		for (int i = 0; i < 4; i++)
		{
			int _texIndex = i * 2;
			col1[i] = _Tex[_texIndex].Sample(SampleType, coord1);
			col2[i] = (input.worldNormal.y>0)?_Tex[_texIndex +1].Sample(SampleType, coord2):_Tex[_texIndex].Sample(SampleType, coord2);
			//col2[i] = _Tex[_texIndex + 1].Sample(SampleType, coord2);
			col3[i] = _Tex[_texIndex].Sample(SampleType, coord3);
			bumpFetch1[i] = _Normal[_texIndex].Sample(SampleType, coord1).xy - 0.5;
			bumpFetch2[i] = (input.worldNormal.y>0) ? _Normal[_texIndex + 1].Sample(SampleType, coord2).xy - 0.5 : _Normal[_texIndex].Sample(SampleType, coord2).xy - 0.5;
			//bumpFetch2[i] = _Normal[_texIndex + 1].Sample(SampleType, coord2).xy - 0.5;
			bumpFetch3[i] = _Normal[_texIndex].Sample(SampleType, coord3).xy - 0.5;
			bump1[i] = float3(0, bumpFetch1[i].x, bumpFetch1[i].y);
			bump2[i] = float3(bumpFetch2[i].y, 0, bumpFetch2[i].x);
			bump3[i] = float3(bumpFetch3[i].x, bumpFetch3[i].y, 0);
		}


		float4 c1=float4(0, 0, 0, 0), c2=float4(0, 0, 0, 0), c3=float4(0, 0, 0, 0);
		float3 b1=float3(0,0,0), b2=float3(0,0,0), b3=float3(0,0,0);


		for (int i = 0; i < 4; i++)
		{
			float p = (i == 0) ? input.color.r:(i == 1) ? input.color.g:(i == 2) ? input.color.b : input.color.a;
			c1 += col1[i].xyzw*p;
			c2 += col2[i].xyzw*p;
			c3 += col3[i].xyzw*p;

			b1 += bump1[i].xyz*p;
			b2 += bump2[i].xyz*p;
			b3 += bump3[i].xyz*p;
		}

		blended_color = c1 * blend_weights.xxxx +
			c2 * blend_weights.yyyy +
			c3 * blend_weights.zzzz;

		//blended_color = blended_color * 2;

		blended_bump_vec = b1 * blend_weights.xxx +
			b2 * blend_weights.yyy +
			b3 * blend_weights.zzz;

		//blended_bump_vec=normalize(blended_bump_vec);
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
		specular = pow(specular, 10.0f);
	}

	float lightDepth = input.lightClipPosition.z / input.lightClipPosition.w;
	float2 shadowUV = input.lightClipPosition.xy / input.lightClipPosition.w;
	shadowUV.y = -shadowUV.y;
	shadowUV = shadowUV * 0.5f + 0.5f;

	float shadowMapDepth = shadowMap.Sample(SampleTypeClamp, shadowUV).r;
	lightDepth -= 0.0002f;//shadowBias;//0.00002f;//



	float3 d = blended_color*diffuseColor*diffuse;
	if (lightDepth > shadowMapDepth)
	{
		d *= 0.3f;
		specular *= 0.3f;
	}
	float3 ambient = ambientColor*0.3f;

	float specularPower = 0.3f;
	return float4(d+ ambient +specular* specularPower,1);
}

