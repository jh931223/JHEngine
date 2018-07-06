cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


struct vInput
{
	float4 position : POSITION;
	float3 normal : NORMAL;
};

struct v2f
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float4 worldPos : TEXCOORD0;
};

Texture2D shaderTexture1;
Texture2D shaderTexture2;
Texture2D shaderTexture3;
SamplerState SampleType;


v2f vs(vInput input)
{
	v2f output;


	input.position.w = 1.0f;
	output.worldPos = mul(input.position, worldMatrix);
	output.position = mul(output.worldPos, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.normal = mul(input.normal, worldMatrix);
	return output;
}

float4 ps(v2f input) : SV_TARGET
{
	float scale = 0.2f;
	float sharpness = 8;
	float3 blending = pow(abs(input.normal), sharpness);
	blending = blending / (blending.x + blending.y + blending.z);

	float4 coords = input.worldPos;
	float4 xaxis = shaderTexture1.Sample(SampleType, coords.yz*scale);
	float4 yaxis = shaderTexture2.Sample(SampleType, coords.xz*scale);
	float4 zaxis = shaderTexture3.Sample(SampleType, coords.xy*scale);
	float4 tex = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
	return tex;
}

