
#include "Header.hlsli"

float GetElevation(float3 pos)
{
     float elevation = 0.0f;

    // Base noise for primary terrain
    elevation += snoise(pos * 0.45f) * 0.65f;

    // Medium freq for hills
    elevation += snoise(pos * 1.5f) * 0.35f;
    // High freq for mountains
    elevation += snoise(pos * 4.0f) * 0.11f;
    // ultra high freq for peaks
    elevation += snoise(pos * 16.0f) * 0.01f;
    
    // Bias everything upwards
    //elevation = pow(elevation, 0.5f);

    // Normalize elevation
    elevation = clamp(elevation, 0.0f, 1.0f);

    return elevation;
}

float3 GetColor(float elevation, float seed)
{
     // Base colors
    float3 deepWaterColor = float3(0.0f, 0.0f, 0.4f);  // Deep Blue
    float3 shallowWaterColor = float3(0.0f, 0.0f, 0.6f); // Lighter Blue
    float3 landColor = float3(0.1f, 0.7f, 0.1f);      // Green
    float3 mountainColor = float3(0.5f, 0.5f, 0.5f); // Gray
    float3 snowColor = float3(1.0f, 1.0f, 1.0f);      // White

    float3 color;

    if (elevation < 0.25f)
    {
        // Water
        float waterFactor = elevation / 0.25f;
        color = lerp(deepWaterColor, shallowWaterColor, waterFactor);
    }
    else if (elevation < 0.55f)
    {
        // Land
        float landFactor = (elevation - 0.25f) / (0.55f - 0.25f);
        color = lerp(landColor, mountainColor, landFactor);
    }
    else
    {
        // Mountains to snow caps
        float mountainFactor = (elevation - 0.55f) / (1.0f - 0.55f);
        color = lerp(mountainColor, snowColor, mountainFactor);
    }

    // Optional: Slight color variation based on seed for uniqueness
    float colorVariation = frac(sin(seed) * 10000.0f) * 0.05f; // Small variation factor
    color += colorVariation;

    return saturate(color); // Ensure color stays within [0,1]
}



struct PS_INPUT
{
 float4 pos        : SV_POSITION;
 float3 WorldPos   : TEXCOORD0;
 float  perlinSeed : TEXCOORD1;
};

float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	//float noiseVal = curl(input.WorldPos.xyz * 2);
    //float3 color = lerp(float3(0,0,1), float3(1,1,1), noiseVal);
    //float noise = snoise(input.WorldPos);
    float seed = input.perlinSeed + 1.0;
    float3 seedOffset = float3(seed * 15.f, seed * 3.14159f, seed * 32.345235235f);

    float elevation = GetElevation(input.WorldPos + seedOffset);
    float3 color = GetColor(elevation, seed);
	//return float4(float3(elevation, elevation, elevation), 1.0);
	return float4(color, 1.0);
 
}