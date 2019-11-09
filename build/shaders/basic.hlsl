cbuffer MatrixBuffer
{
    float4x4 world;
	float4x4 view;
    float4x4 worldViewProj;
    float3 lightDir;
};

struct VertexInputType
{
	float4 position : POSITION; 
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 positionCS : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

PixelInputType VShader(VertexInputType input)
{
    PixelInputType output;
    
	float4 normalWS = mul(world, float4(input.normal, 0.0));
	float diffuse = saturate(dot(normalWS.xyz, lightDir));
	
    output.position = mul(worldViewProj, input.position);
    output.color = float4(0.5, 0.5, 0.5, 1.0) + float4(diffuse * 0.5, diffuse * 0.5, diffuse * 0.5, 1.0);
	output.normal = normalWS.xyz;
	output.uv = input.uv;
	output.positionCS = output.position;
	
    return output;
}


Texture2D shaderTexture;
SamplerState SampleType;

float4 PShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor = shaderTexture.Sample(SampleType, input.uv);

	return textureColor * input.color;
	
	//float n = 0.5;
	//float f = 500.0;
	//float z = input.positionCS.z / input.positionCS.w;
	//
	//
	//float LZ = n * (z + 1.0) / (f + n - z * (f - n));
	//
	//return float4(LZ, LZ, LZ, 0.0);
    //return color;
	//float3 normalColor = input.normal * 0.5 + float3(0.5, 0.5, 0.5);
	//return float4(normalColor.x, normalColor.y, normalColor.z, 1.0);
}