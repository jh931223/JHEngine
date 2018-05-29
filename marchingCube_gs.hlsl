struct Vert
{
	float3 position;
	float2 uv;
	float3 normal;
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}