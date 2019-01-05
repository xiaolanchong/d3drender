struct PSInput
{
    float4 color : COLOR;
    float2 uv    : TEXCOORD0;
};

float3 chromaKeyColor : register(c0);

SamplerState textureSampler;

float4 main(PSInput input) : SV_TARGET
{
    float4 color = tex2D(textureSampler, input.uv);
    float3 diff = color.rgb-chromaKeyColor;
	float sqr = dot(diff, diff);
	color.a = sqr;
    return color;
}