cbuffer MatrixBuffer : register(b0)
{
    float4x4 world;
	float4x4 view;
    float4x4 worldViewProj;
    float3 lightDir;
};

struct VertexInputType
{
	float4 position : POSITION;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
};

PixelInputType VShader(VertexInputType input)
{
    PixelInputType output;
    
    output.position = mul(worldViewProj, input.position);
	
    return output;
}

float4 PShader(PixelInputType input) : SV_TARGET
{
	return float4(0);
}