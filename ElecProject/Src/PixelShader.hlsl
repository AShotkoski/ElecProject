
#include "Header.hlsli"

float GetElevation(float3 pos)
{
    // Parameters for noise generations
    // For noise, higher is more noisy
    // For scale, constitutes the max elevation from this step
    const float first_noise = 0.1f;
    const float first_scale = 25.f;
    const float second_noise = 0.175f;
    const float second_scale = 15.f;
    const float third_noise = 0.6f;
    const float third_scale = 5.f;
    const float fourth_noise = 4.0f;
    const float fourth_scale = 0.5f;
    const float elevation_max = first_scale + second_scale + third_scale + fourth_scale;
    

     float elevation = 0.0f;
    
    // Base noise for primary terrain
    elevation += snoise(pos * first_noise) * first_scale;
    
    // Medium freq for hills
    elevation += snoise(pos * second_noise) * second_scale;
    // High freq for mountains
    elevation += snoise(pos * third_noise) * third_scale;
    // ultra high freq for peaks
    elevation += snoise(pos * fourth_noise) * fourth_scale;
    
    // If all noise funcs return -1 the lowest elevation is elevation_max
    // this this makes all the values pos
    elevation += elevation_max;
    // Normalize the elevation 
    elevation /= (2.f * elevation_max);

    // Normalize elevation
    elevation = clamp(elevation, 0.0f, 1.0f);

    return elevation;
}


float3 interpolation(float3 a, float3 b, float x)
{
    float amount = pow(x,3);
    return lerp(a,b,amount);
}

float3 GetColor(float elevation, float seed)
{
     // Base colors
    const float3 deepWaterColor = float3(0.0f, 0.0f, 0.25f);  // Deep Blue
    const float3 shallowWaterColor = float3(0.0f, 0.0f, 0.6f); // Lighter Blue
    const float3 landColor = float3(0.1f, 0.7f, 0.1f);      // Green
    const float3 mountainColor = float3(0.5f, 0.5f, 0.5f); // Gray
    const float3 snowColor = float3(1.0f, 1.0f, 1.0f);      // White

    // Lots of variables
    const float waterlevel = 0.5f;
    const float mountainlevel = 0.75f;


    float3 color;

    if (elevation < waterlevel)
    {
        // Water
        float waterFactor = elevation / waterlevel;
        color = interpolation(deepWaterColor, shallowWaterColor, waterFactor);
    }
    else if (elevation < mountainlevel)
    {
        // Land
        float landFactor = (elevation - waterlevel) / (mountainlevel - waterlevel);
        color = lerp(landColor, mountainColor, landFactor);
    }
    else
    {
        // Mountains to snow caps
        float mountainFactor = (elevation - mountainlevel) / (1.0f - mountainlevel);
        color = interpolation(mountainColor, snowColor, mountainFactor);
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