cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct vInput
{
	float4 position : POSITION;
};
struct v2f
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXCOORD0;
};
v2f main(vInput input)
{
	v2f output;
	float4 worldPosition = mul(input.position, worldMatrix);
	worldPosition.w = 1.0f;
	output.position = mul(worldPosition, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.depthPosition = output.position;
	return output;
}

float4 ps(v2f input) : SV_TARGET
{
	float depth = input.depthPosition.z / input.depthPosition.w;
	return float4(depth, 0, 0, 1);
}