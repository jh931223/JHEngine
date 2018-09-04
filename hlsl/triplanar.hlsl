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

Texture2D shaderTexture1 : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D shaderTexture3 : register(t2);
Texture2D stn1 : register(t3);
Texture2D stn2 : register(t4);
Texture2D stn3 : register(t5);

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

	float3 worldNormal = mul(normalize(input.normal), worldMatrix);
	output.worldNormal = worldNormal;

	float3 lDir = normalize(lightDir);
	output.diffuse = dot(worldNormal, -lDir);
	output.reflection = reflect(lDir, worldNormal);
	return output;
}

float3x3 cotangent_frame(float3 N, float3 p, float2 uv)
{
	// get edge vectors of the pixel triangle
	float3 dp1 = ddx(p);
	float3 dp2 = ddy(p);
	float2 duv1 = ddx(uv);
	float2 duv2 = ddy(uv);

	// solve the linear system
	float3 dp2perp = cross(dp2, N);
	float3 dp1perp = cross(N, dp1);
	float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// Negate because of left-handedness
	//T *= -1;
	//B *= -1;

	// construct a scale-invariant frame 
	float invmax = rsqrt(max(dot(T, T), dot(B, B)));
	return float3x3(T * invmax, B * invmax, N);
}

/** Magic TBN-Calculation function */
float3 perturb_normal(float3 N, float3 V, float2 texcoord, Texture2D tex, float normalmapDepth = 1.0f)
{
	// assume N, the interpolated vertex normal and 
	// V, the view vector (vertex to eye)
	float3 nrmmap = tex.Sample(SampleType, texcoord).xyz * 2 - 1;
	nrmmap.xy *= -1.0f;
	nrmmap.xy *= normalmapDepth;
	nrmmap = normalize(nrmmap);

	float3x3 TBN = cotangent_frame(N, -V, texcoord);
	return normalize(mul(transpose(TBN), nrmmap));
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
		specular = pow(specular, 1.3f);
	}

	float scale =10.0f;
	float sharpness = 1.3f;


	float4 coords = input.worldPos;

	float2 xUV = coords.zy / scale;
	float2 yUV = coords.xz / scale;
	float2 zUV = coords.xy / scale;



	float3 blending = pow(abs(input.worldNormal), sharpness);
	blending = blending / (blending.x + blending.y + blending.z);


	float4 xaxis = shaderTexture1.Sample(SampleType, xUV);
	float4 yaxis = shaderTexture2.Sample(SampleType, yUV);
	float4 zaxis = shaderTexture3.Sample(SampleType, zUV);

	/*float4 xaxisN = stn1.Sample(SampleType, coords.yz / scale);
	float4 yaxisN = stn2.Sample(SampleType, coords.xz / scale);
	float4 zaxisN = stn3.Sample(SampleType, coords.xy / scale);*/

	float3 n1 = perturb_normal(input.worldNormal, input.worldPos, xUV, stn1,3);
	float3 n2 = perturb_normal(input.worldNormal, input.worldPos, yUV, stn2,3);
	float3 n3 = perturb_normal(input.worldNormal, input.worldPos, zUV, stn3,3);


	float3 normal = n1 * blending.x + n2 * blending.y + n3 * blending.z;


	float3 blending2 = pow(abs(normal), sharpness);
	blending2 = blending2 / (blending2.x + blending2.y + blending2.z);

	float3 albedo = xaxis * blending2.x + yaxis * blending2.y + zaxis * blending2.z;


	float3 d = albedo * diffuse*diffuseColor;
	float specularPower = 0.2f;
	return float4(d+ambientColor+specular* specularPower,1);
}

