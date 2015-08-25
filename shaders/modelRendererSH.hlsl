
cbuffer ConstantBuffer : register(b0)
{
	matrix worldViewProj;
	float4 modelColor;
}

cbuffer ConstantBuffer : register(b1)
{
	float lightingCoefficients[9];
}

#ifdef TEXTURE
Texture2D modelTexture : register(t0);
SamplerState modelSampler : register(s0);
#endif

struct VertexShaderOutput
{
	float4 position : SV_POSITION;
#ifdef TEXTURE
	float2 texCoord : TEXCOORD0;
#else
	float4 color : TEXCOORD0;
#endif
	float3 normal : NORMAL;
	float3 worldPos : WORLDPOS;
};

VertexShaderOutput vertexShaderMain(float4 position : position,
	float3 normal : normal,
	float4 color : color,
	float2 texCoord : texCoord)
{
	VertexShaderOutput output;
	output.position = mul(position, worldViewProj);
#ifdef TEXTURE
	output.texCoord = texCoord.xy;
#else
	output.color = color;
#endif
	output.normal = normal;
	output.worldPos = position.xyz;
	return output;
}


float evaluateLightingModel(const in float3 n)
{
	float H[9];

	H[0] = 1.0f;
	H[1] = n[1];
	H[2] = n[2];
	H[3] = n[0];
	H[4] = n[0] * n[1];
	H[5] = n[1] * n[2];
	H[6] = 3.0f * n[2] * n[2] - 1.0f;
	H[7] = n[2] * n[0];
	H[8] = n[0] * n[0] - n[1] * n[1];


	float res = 0.0f;
	for (unsigned int i = 0; i < 9; i++) {
		res += H[i] * lightingCoefficients[i];
	}

	return res;
}


float4 pixelShaderMain(VertexShaderOutput input) : SV_Target
{
	float shA = evaluateLightingModel(normalize(input.normal));
    float shB = evaluateLightingModel(normalize(-input.normal));

    float sh = max(max(shA, shB), 0.2);

    //return float4(sh, sh, sh, 1.0f);

#ifdef TEXTURE
	float4 texColor = modelTexture.Sample(modelSampler, float2(input.texCoord.x, 1.0 - input.texCoord.y));
	return float4(sh * texColor.xyz * modelColor.xyz, 1.0f);
#else 
	return float4(sh * modelColor.xyz, 1.0f);
#endif

	//float3 lightDir = normalize(float3(1.0, -2.0, 3.0));
	//float3 lightColor = abs(dot(normalize(input.normal), lightDir)) * float3(0.7, 0.7, 0.7) + float3(0.3, 0.3, 0.3);
	//return float4(lightColor * modelColor.xyz * input.color.xyz, 1.0f);

}


