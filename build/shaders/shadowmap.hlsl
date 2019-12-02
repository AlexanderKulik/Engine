cbuffer MatrixBuffer : register(b0)
{
    float4x4 worldViewProj;
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
    
    output.position = mul(float4(input.position.xyz, 1), worldViewProj);
	
    return output;
}

float4 PShader(PixelInputType input) : SV_TARGET
{
	return float4(0, 0, 0, 0);
}