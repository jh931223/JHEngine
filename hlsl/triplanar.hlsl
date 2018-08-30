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
	float diffuse : TEXCOORD2;
	float3 viewDir : TEXCOORD3;
	float3 reflection : TEXCOORD4;
};

Texture2D shaderTexture1;
Texture2D shaderTexture2;
Texture2D shaderTexture3;
SamplerState SampleType;


v2f vs(vInput input)
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

	float3 worldNormal = mul(input.normal, worldMatrix);
	output.worldNormal = worldNormal;

	float3 lDir = normalize(lightDir);
	output.diffuse = dot(worldNormal, -lDir);
	output.reflection = reflect(lDir, worldNormal);
	return output;
}

float4 ps(v2f input) : SV_TARGET
{
//	float4 tex = float4(input.normal,1);
	float diffuse = saturate(input.diffuse);
	float3 reflection = normalize(input.reflection);
	float3 viewDir = normalize(input.viewDir);
	float3 specular = 0;

	if (diffuse > 0)
	{
		specular = saturate(dot(reflection, -viewDir));
		specular = pow(specular, 20.0f);
	}

	float scale = 0.2f;
	float sharpness = 1.3;
	float3 blending = pow(abs(input.worldNormal), sharpness);
	blending = blending / (blending.x + blending.y + blending.z);

	float4 coords = input.worldPos;
	float4 xaxis = shaderTexture1.Sample(SampleType, coords.yz*scale);
	float4 yaxis = shaderTexture2.Sample(SampleType, coords.xz*scale);
	float4 zaxis = shaderTexture3.Sample(SampleType, coords.xy*scale);
	float3 tex = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
	float3 albedo = tex * diffuse*diffuseColor;
	float specularPower = 0.2f;
	return float4(albedo+ambientColor+specular* specularPower,1);
}

