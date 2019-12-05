cbuffer MatrixBuffer
{
    float4x4 world;
	float4x4 view;
    float4x4 worldViewProj;
    float4x4 shadowMapMatrix;
    float3 lightDir;
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
	float4 shadowMapSpace : SHADOWMAP_SPACE;
    float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float fogFactor : FOGFACTOR;
    float NdotL : DIFFUSE;
};

PixelInputType VShader(VertexInputType input)
{
    PixelInputType output;
    
	float4 normalWS = mul(float4(input.normal, 0), world);
	float4 positionVS = mul(float4(input.position, 1), view);

    output.position = mul(float4(input.position, 1), worldViewProj);
	output.shadowMapSpace = mul(float4(input.position, 1), mul(world, shadowMapMatrix));
	output.normal = normalWS.xyz;
	output.uv = input.uv;
	output.fogFactor = saturate( (positionVS.z - fogParams.x)/(fogParams.y - fogParams.x) );
    output.NdotL = saturate(dot(normalWS.xyz, lightDir));
	
    return output;
}


Texture2D shaderTexture;
SamplerState SampleType;

Texture2D shadowMapSampler;
SamplerState shadowMapSamplerState;

float4 PShader(PixelInputType input) : SV_TARGET
{
	float3 output;
	
	float shadow = 1.0;
	float4 shadowMapCoords = input.shadowMapSpace / input.shadowMapSpace.w;
	
	if (shadowMapCoords.x > -1 && shadowMapCoords.x < 1 && shadowMapCoords.y > -1 && shadowMapCoords.y < 1)
	{
		float shadowMapDepth = shadowMapSampler.Sample(shadowMapSamplerState, shadowMapCoords.xy * float2(0.5, -0.5) + float2(0.5, 0.5));
		if (shadowMapDepth + 0.005 < shadowMapCoords.z)
		{
			shadow = 0.0;
		}
	}

	float4 textureColor = float4(1, 1, 1, 1); // shaderTexture.Sample(SampleType, input.uv);

	output.rgb = textureColor * (input.NdotL * shadow * 0.8 + 0.2);

	output.rgb = lerp(output.rgb, fogColor.rgb, input.fogFactor * fogColor.a);

	return float4(output, 1);
}