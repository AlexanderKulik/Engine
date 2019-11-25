cbuffer MatrixBuffer : register(b0)
{
    float4x4 world;
	float4x4 view;
    float4x4 worldViewProj;
    float3 lightDir;
};

cbuffer FogData : register(b1)
{
	float4 fogParams;
	float4 fogColor;
};

struct VertexInputType
{
	float3 position : POSITION; 
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float fogFactor : FOGFACTOR;
};

PixelInputType VShader(VertexInputType input)
{
    PixelInputType output;
    
	float4 normalWS = mul(world, float4(input.normal, 0.0));
	float diffuse = saturate(dot(normalWS.xyz, lightDir));
	
	float4 positionVS = mul(view, float4(input.position, 1));
	float fogFactor = saturate( (positionVS.z - fogParams.x)/(fogParams.y - fogParams.x) );

    output.position = mul(worldViewProj, float4(input.position, 1));
    output.color = float4(0.5, 0.5, 0.5, 1.0) + float4(diffuse * 0.5, diffuse * 0.5, diffuse * 0.5, 1.0);
	output.normal = normalWS.xyz;
	output.uv = input.uv;
	output.fogFactor = fogFactor;
	
    return output;
}


Texture2D shaderTexture;
SamplerState SampleType;

float4 PShader(PixelInputType input) : SV_TARGET
{
	float3 output;

	float4 textureColor = shaderTexture.Sample(SampleType, input.uv);

	output.rgb = textureColor * input.color;

	output.rgb = lerp(output.rgb, fogColor.rgb, input.fogFactor * fogColor.a);

	return float4(output, 1);
}