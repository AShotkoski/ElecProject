// Hash function to generate pseudo-random values
float Hash(float3 p)
{
    p = frac(p * 0.3183099 + 0.1);
    p *= 17.0;
    return frac(p.x * p.y * p.z * (p.x + p.y + p.z));
}

// Noise function using trilinear interpolation
float Noise(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);

    // Hash the corners of the cube
    float n000 = Hash(i + float3(0, 0, 0));
    float n001 = Hash(i + float3(0, 0, 1));
    float n010 = Hash(i + float3(0, 1, 0));
    float n011 = Hash(i + float3(0, 1, 1));
    float n100 = Hash(i + float3(1, 0, 0));
    float n101 = Hash(i + float3(1, 0, 1));
    float n110 = Hash(i + float3(1, 1, 0));
    float n111 = Hash(i + float3(1, 1, 1));

    // Smooth interpolation
    float3 u = f * f * (3.0 - 2.0 * f);

    // Interpolate along x, y, then z
    return lerp(
        lerp(
            lerp(n000, n100, u.x),
            lerp(n010, n110, u.x), u.y),
        lerp(
            lerp(n001, n101, u.x),
            lerp(n011, n111, u.x), u.y),
        u.z);
}

// Fractal noise function to sum multiple octaves
float FractalNoise(float3 p)
{
    float total = 0.0;
    float frequency = 1.0;
    float amplitude = 1.0;
    float maxAmplitude = 0.0;

    // Loop through octaves
    for (int i = 0; i < 5; ++i)
    {
        total += Noise(p * frequency) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return total / maxAmplitude; // Normalize the result
}


struct PS_INPUT
{
 float4 pos : SV_POSITION;
 float3 WorldPos : TEXCOORD0;
};

float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	float noiseVal = FractalNoise(input.WorldPos * 10);
	return float4(noiseVal, noiseVal, noiseVal, 1.0);
}