cbuffer MatrixBuffer : register(b0)
{
    matrix world;
    matrix worldViewProj;
};
struct VS_INPUT
{
    float3 pos : POSITION;
};
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
};
PS_INPUT VS_Main(VS_INPUT input)
{
    PS_INPUT output;
    // Transform pos to clip space
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    output.WorldPos = mul(float4(input.pos, 1.0f), world).xyz;
    return output;
};