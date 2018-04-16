////////////////////////////////////////////////////////////////////////////////
// Filename: depth_ps.hlsl
////////////////////////////////////////////////////////////////////////////////


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float depthValue;
float4 color;


// Z �ȼ� ���̸� ���� W ��ǥ�� ������ �ȼ��� ���� ���� �����ɴϴ�.
depthValue = input.depthPosition.z / input.depthPosition.w;

// ���� ������ ó�� 10 %�� �������Դϴ�.
if (depthValue < 0.9f)
{
	color = float4(1.0, 0.0f, 0.0f, 1.0f);
}

// ���� ������ ���� 0.025 % �κ��� ������� ǥ�õ˴ϴ�.
if (depthValue > 0.9f)
{
	color = float4(0.0, 1.0f, 0.0f, 1.0f);
}

// ���� ������ �������� �Ķ������� ǥ�õ˴ϴ�.
if (depthValue > 0.925f)
{
	color = float4(0.0, 0.0f, 1.0f, 1.0f);
}

return color;
}
