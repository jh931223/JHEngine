cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer PSLIghtBuffer
{
	float4 mainColor;
};

struct vInput
{
	float4 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct v2f
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D MainTex : register(t0);;
SamplerState SampleType;

v2f main(vInput input)
{
	v2f output;
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.uv = input.uv;
	return output;
}

float4 ps(v2f input) : SV_TARGET
{
	float4 rgb= MainTex.Sample(SampleType, input.uv)*mainColor;
	return float4(rgb.r,rgb.g,rgb.b,mainColor.a);
}