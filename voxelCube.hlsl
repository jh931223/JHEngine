
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
	o.normal = edgeNormal[0];
	outStream.Append(o);

	o.position = pos + float4(-u, 0, u, 0);
	o.position = mul(o.position, worldMatrix);
	o.position = mul(o.position, viewMatrix);
	o.position = mul(o.position, projectionMatrix);
	o.uv = float2(0, 0);
	o.normal = edgeNormal[0];
	outStream.Append(o);

	o.position = pos + float4(u, 0, u, 0);
	o.position = mul(o.position, worldMatrix);
	o.position = mul(o.position, viewMatrix);
	o.position = mul(o.position, projectionMatrix);
	o.uv = float2(1, 0);
	o.normal = edgeNormal[0];
	outStream.Append(o);

	outStream.RestartStrip();
}


[maxvertexcount(15)]
void gs(point v2g input[1], inout TriangleStream<g2f> outStream)
{
	float4 pos = input[0].position;
	float3 temp = pos.xyz - startPosition;
	int vid = (int)temp.x + (int)temp.y*length + (int)temp.z*length*length;
	int _case = mcData[vid];
	if (_case == 0 || _case == 255)
	{
		return;
	}
	float offset = 1.0f * 0.5f;
	float3 _verts[12];// = { float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0),float3(0,0,0) };
	int i, j;
	for (i = 0; i < 12; i++)
	{
		if ((edgeTable[_case] & (1 << i)) != 0)
		{
			_verts[i] = pos.xyz + mcVertexOffset[i] * offset;
		}
	}
	for (i = 0; i < 5; i++)
	{
		int t = triTable[_case][i * 3];
		if (t<0)
		{
			break;
		}
		float4 pos0, pos1, pos2;
		pos0 = float4(_verts[triTable[_case][(i * 3)]], 1);
		pos1 = float4(_verts[triTable[_case][(i * 3 + 1)]], 1);
		pos2 = float4(_verts[triTable[_case][(i * 3 + 2)]], 1);
		float3 normal = GenerateNormal(pos0, pos1, pos2);
		for (j = 2; j >= 0; j--)
		{
			g2f o;
			int edge = triTable[_case][(i * 3 + j)];
			o.uv = (j == 0) ? float2(0, 0) : (j == 1) ? float2(1 * 0.5f, 0) : float2	(0, 1);
			o.position = float4(_verts[edge].xyz, 1);
			o.worldPos = mul(o.position, worldMatrix);
			o.position = mul(o.worldPos, viewMatrix);
			o.position = mul(o.position, projectionMatrix);
			o.normal = mul(normal, worldMatrix);// edgeNormal[edge];
			outStream.Append(o);
		}

		outStream.RestartStrip();
	}
	//for (i = 0; i < 5; i++)
	//{
	//	int t = triTable[_case][i * 3];
	//	if (t<0)
	//	{
	//		break;
	//	}
	//	for (j = 2; j >= 0; j--)
	//	{
	//		g2f o;
	//		int edge = triTable[_case][(i * 3 + j)];
	//		o.uv = (j == 0) ? float2(0, 0) : (j == 1) ? float2(1 * 0.5f, 0) : float2(0, 1);
	//		o.position = float4(_verts[edge].xyz, 1);
	//		o.position = mul(o.position, worldMatrix);
	//		o.position = mul(o.position, viewMatrix);
	//		o.position = mul(o.position, projectionMatrix);
	//		o.normal = edgeNormal[edge];
	//		outStream.Append(o);
	//	}
	//	outStream.RestartStrip();
	//}
}
