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
	float3 viewDir : TEXCOORD2;
	float3 lightDir : TEXCOORD3;
};

Texture2D shaderTexture1 : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D shaderTexture3 : register(t2);
Texture2D stn1 : register(t3);
Texture2D stn2 : register(t4);
Texture2D stn3 : register(t5);

SamplerState SampleType;


v2f main(vInput input)
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
	output.lightDir = lDir;
	return output;
}

float4 ps(v2f input) : SV_TARGET
{
	// Determine the blend weights for the 3 planar projections.
	// N_orig is the vertex-interpolated normal vector.
	float3 blend_weights = abs(input.worldNormal.xyz);   // Tighten up the blending zone:
	float tex_sharpness = 2.0f;
	blend_weights = (blend_weights - 0.2) * tex_sharpness;
	blend_weights = max(blend_weights, 0);      // Force weights to sum to 1.0 (very important!)
	blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z).xxx;
	// Now determine a color value and bump vector for each of the 3
	// projections, blend them, and store blended results in these two
	// vectors:
	float4 blended_color; // .w hold spec value
	float3 blended_bump_vec;
	{
		// Compute the UV coords for each of the 3 planar projections.
		// tex_scale (default ~ 1.0) determines how big the textures appear.
		float tex_scale = 0.1f;
		float2 coord1 = input.worldPos.yz * tex_scale;
		float2 coord2 = input.worldPos.zx * tex_scale;
		float2 coord3 = input.worldPos.xy * tex_scale;
		// This is where you would apply conditional displacement mapping.
		//if (blend_weights.x > 0) coord1 = . . .
		//if (blend_weights.y > 0) coord2 = . . .
		//if (blend_weights.z > 0) coord3 = . . .
		// Sample color maps for each projection, at those UV coords.
		float4 col1 = shaderTexture1.Sample(SampleType,coord1);
		float4 col2 = shaderTexture2.Sample(SampleType, coord2);
		float4 col3 = shaderTexture3.Sample(SampleType, coord3);
		// Sample bump maps too, and generate bump vectors.
		// (Note: this uses an oversimplified tangent basis.)
		float2 bumpFetch1 = stn1.Sample(SampleType, coord1).xy - 0.5;
		float2 bumpFetch2 = stn2.Sample(SampleType, coord2).xy - 0.5;
		float2 bumpFetch3 = stn3.Sample(SampleType, coord3).xy - 0.5;
		float3 bump1 = float3(0, bumpFetch1.x, bumpFetch1.y);
		float3 bump2 = float3(bumpFetch2.y, 0, bumpFetch2.x);
		float3 bump3 = float3(bumpFetch3.x, bumpFetch3.y, 0);
		// Finally, blend the results of the 3 planar projections.
		blended_color = col1.xyzw * blend_weights.xxxx +
			col2.xyzw * blend_weights.yyyy +
			col3.xyzw * blend_weights.zzzz;
		blended_bump_vec = bump1.xyz * blend_weights.xxx +
			bump2.xyz * blend_weights.yyy +
			bump3.xyz * blend_weights.zzz;
	}
	// Apply bump vector to vertex-interpolated normal vector.
	float3 N_for_lighting = normalize(input.worldNormal + blended_bump_vec);

	
	float diffuse = dot(N_for_lighting, -input.lightDir);
	float3 reflection = reflect(input.lightDir, N_for_lighting);

	diffuse = saturate(diffuse);
	reflection = normalize(reflection);

	float3 viewDir = normalize(input.viewDir);
	float3 specular = 0;

	if (diffuse > 0)
	{
		specular = saturate(dot(reflection, -viewDir));
		specular = pow(specular, 1.3f);
	}



	float3 d = blended_color * diffuse*diffuseColor;
	float specularPower = 0.2f;
	return float4(d+ambientColor+specular* specularPower,1);
}

