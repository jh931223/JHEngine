cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};
cbuffer LightBuffer
{
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
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
	input.position.w = 1.0f;
	float4 worldPosition = mul(input.position, worldMatrix);
	output.position = mul(worldPosition, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.depthPosition = mul(worldPosition, lightViewMatrix);
	output.depthPosition = mul(output.depthPosition, lightProjectionMatrix);
	return output;
}

float4 ps(v2f input) : SV_TARGET
{
	float depth = input.depthPosition.z / input.depthPosition.w;
	return float4(depth,depth,depth, 1);
}