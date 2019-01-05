struct PSInput
{
    float4 color : COLOR;
    float2 uv    : TEXCOORD0;
};

SamplerState textureSampler;

float4 main(PSInput input) : SV_TARGET
{
    float4 color = tex2D(textureSampler, input.uv);
    // color.rgb = dot(color.rgb, color.rgb);
    color.a = 1;
    return color;
}