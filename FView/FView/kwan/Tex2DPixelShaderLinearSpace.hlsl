Texture2D colorMap_ : register(t0);
SamplerState colorSampler_ : register(s0);

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 texR : TEXCOORD0;
};

float4 PS_Main(PS_Input frag) : SV_TARGET
{
    ///////Gama space to Linner space
    float4 color = colorMap_.Sample(colorSampler_, frag.texR);
    color.xyz = max(color.xyz, half3(0.h, 0.h, 0.h));
    color.xyz = max(1.055h * pow(color.xyz, 0.416666667h) - 0.055h, 0.h);
    ////////////////////
    return color;
}