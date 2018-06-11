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


// Z 픽셀 깊이를 균질 W 좌표로 나누어 픽셀의 깊이 값을 가져옵니다.
depthValue = input.depthPosition.z / input.depthPosition.w;

// 깊이 버퍼의 처음 10 %는 빨간색입니다.
if (depthValue < 0.9f)
{
	color = float4(1.0, 0.0f, 0.0f, 1.0f);
}

// 깊이 버퍼의 다음 0.025 % 부분이 녹색으로 표시됩니다.
if (depthValue > 0.9f)
{
	color = float4(0.0, 1.0f, 0.0f, 1.0f);
}

// 깊이 버퍼의 나머지는 파란색으로 표시됩니다.
if (depthValue > 0.925f)
{
	color = float4(0.0, 0.0f, 1.0f, 1.0f);
}

return color;
}
