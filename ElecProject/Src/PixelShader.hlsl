struct PS_INPUT
{
float4 pos : SV_POSITION;
 float3 WorldPos : TEXCOORD0;
};

float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	float dist = length(input.pos.xyz) * 0.005;
	float col = saturate(dist); 
	return float4(col,1 - col,col,1);
}