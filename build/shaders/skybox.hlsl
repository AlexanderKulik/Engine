cbuffer MatrixBuffer : register(b0)
{
    float4x4 world;
	float4x4 view;
    float4x4 worldViewProj;
    float3 lightDir;
};

struct VertexInputType
{
	float3 position : POSITION;
};

struct PixelInputType
{
    float3 uv : TEXCOORD;
};

PixelInputType VShader(VertexInputType input)
{
    PixelInputType output;
    
    output.position = mul(worldViewProj, float4(input.position, 1));
    output.color = input.color;
	
    return output;
}

TextureCube skyboxTexture;
SamplerState SampleType;


float4 PShader(PixelInputType input) : SV_TARGET
{
	return input.color;
}