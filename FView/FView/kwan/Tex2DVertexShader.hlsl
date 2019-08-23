//顶点着色器文件

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
}
cbuffer ProjectBuffer : register(b1)
{
    matrix View;
    matrix Project;
}

struct VS_Input
{
    float4 pos : POSITION;
    float2 InputTexcoord : TEXCOORD0;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 texR : TEXCOORD0;
};

PS_Input VS_Main(VS_Input vertex)
{
    PS_Input vsOut = (PS_Input)0;
    vsOut.pos = mul(vertex.pos, World);
    vsOut.pos = mul(vsOut.pos, View);
    vsOut.pos = mul(vsOut.pos, Project);
    vsOut.texR = vertex.InputTexcoord;

    return vsOut;
}
