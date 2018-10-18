cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};

struct vInput
{
	float4 position : POSITION;
}
struct v2f
{
	float4 position : SV_POSITION;
}
v2f main(vInput input)
{
	v2f output;
	output.position = mul(output.position, worldMatrix);
	output.position = mul(output.position, lightViewMatrix);
	output.position = mul(output.position, lightProjectionMatrix);
	return output;
}

float4 ps(v2f input)
{
	return float4(1, 1, 1, 1);
}