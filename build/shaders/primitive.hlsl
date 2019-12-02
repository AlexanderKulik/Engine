cbuffer MatrixBuffer : register(b0)
{
    float4x4 worldViewProj;
};

struct VertexInputType
{
	float3 position : POSITION; 
	float4 color : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PixelInputType VShader(VertexInputType input)
{
    PixelInputType output;
    
    output.position = mul(float4(input.position, 1), worldViewProj);
    output.color = input.color;
	
    return output;
}


float4 PShader(PixelInputType input) : SV_TARGET
{
	return input.color;
}