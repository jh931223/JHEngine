
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CubeBuffer
{
	float3 startPosition;
	float length;
};


struct vInput
{
	float4 position : POSITION;
	uint vertexID : SV_VertexID;
};

struct v2g
{
	float4 position : POSITION;
	uint vertexID : VertexID;
};

struct g2f
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float4 worldPos : TEXCOORD1;
	//float4 color : TEXCOORD1;
};

StructuredBuffer<int> chunkData;


Texture2D shaderTexture1;
Texture2D shaderTexture2;
Texture2D shaderTexture3;
SamplerState SampleType;



float3 GenerateNormal(float4 pos0, float4 pos1, float4 pos2)
{
	float3 v1 = pos0.xyz - pos1.xyz;
	float3 v2 = pos2.xyz - pos1.xyz;
	return normalize(cross(v1, v2));
}

v2g vs(vInput input)
{
	v2g output;


	input.position.w = 1.0f;

	output.position = input.position;
	output.vertexID = input.vertexID;
	return output;
}

float4 ps(g2f input) : SV_TARGET
{

	float scale = 0.2f;
	float sharpness = 5;
	float3 blending = pow(abs(input.normal),0.0001);
	blending = blending / (blending.x + blending.y + blending.z);

	float4 coords = input.worldPos;
	float4 xaxis = shaderTexture1.Sample(SampleType, coords.yz*scale);
	float4 yaxis = shaderTexture2.Sample(SampleType, coords.xz*scale);
	float4 zaxis = shaderTexture3.Sample(SampleType, coords.xy*scale);
	float4 tex = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
	return tex;
}



[maxvertexcount(8)]
void gs(point v2g input[1], inout TriangleStream<g2f> outStream)
{

	g2f o;
	float4 pos = input[0].position;
	float u = 0.5f;
	o.position = pos + float4(-u, 0, -u,0);
	o.position = mul(o.position, worldMatrix);
	o.position = mul(o.position, viewMatrix);
	o.position = mul(o.position, projectionMatrix);
	o.uv = float2(0, 1);
	outStream.Append(o);

	o.position = pos + float4(-u, 0, u, 0);
	o.position = mul(o.position, worldMatrix);
	o.position = mul(o.position, viewMatrix);
	o.position = mul(o.position, projectionMatrix);
	o.uv = float2(0, 0);
	outStream.Append(o);

	o.position = pos + float4(u, 0, u, 0);
	o.position = mul(o.position, worldMatrix);
	o.position = mul(o.position, viewMatrix);
	o.position = mul(o.position, projectionMatrix);
	o.uv = float2(1, 0);
	outStream.Append(o);

	outStream.RestartStrip();
}
