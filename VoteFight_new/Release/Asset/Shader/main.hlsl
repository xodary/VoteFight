cbuffer CB_GameFramework : register(b0)
{
    float gfTotalTime : packoffset(c0);
    float gfElapsedTime : packoffset(c0.y);
};

cbuffer CB_Camera : register(b1)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);

    float3 gvCameraPosition : packoffset(c8);
};

struct Fog
{
    matrix m_color;
    float m_density;
};

struct LIGHT
{
    bool m_isActive;
			   
    float3 m_position;
    float3 m_direction;
			   
    int m_type;
			   
    float4 m_color;
			   
    float3 m_attenuation;
    float m_fallOff;
    float m_range;
    float m_theta;
    float m_phi;
			   
    bool m_shadowMapping;
    matrix m_toTexCoord;
};

#define MAX_LIGHTS 3
cbuffer CB_Light : register(b2)
{
    LIGHT m_lights[MAX_LIGHTS];
    Fog m_fog;
};

cbuffer CB_Object : register(b3)
{
    matrix gmtxGameObject : packoffset(c0);
    float4 gvColor : packoffset(c4);
    int gnTexturesMask : packoffset(c5);
    float2 gvTextureScale : packoffset(c5.y);
};

cbuffer CB_Sprite : register(b4)
{
    float width : packoffset(c0);
    float height : packoffset(c0.y);
    float left : packoffset(c0.z);
    float top : packoffset(c0.w);
};

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsetInfo : register(b5)
{
    matrix gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransformInfo : register(b6)
{
    matrix gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_NORMAL_MAP			0x02

Texture2D gtxtAlbedoTexture : register(t0);
Texture2D gtxtNormalTexture : register(t1);
TextureCube gtxtCubeTexture : register(t2);

SamplerState samplerState : register(s0);
SamplerState pcfSamplerState : register(s1);
SamplerState gssClamp : register(s2);

struct VS_POSITION_INPUT
{
    float3 position : POSITION;
};

struct VS_POSITION_OUTPUT
{
    float4 position : SV_POSITION;
};

// ======== Depth Write =========
VS_POSITION_OUTPUT VS_Position(VS_POSITION_INPUT input)
{
    VS_POSITION_OUTPUT output;
    
    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);

    return (output);
}

struct VS_POSITION_SKINNING_INPUT
{
    float3 position : POSITION;
    int4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

VS_POSITION_OUTPUT VS_Position_Skinning(VS_POSITION_SKINNING_INPUT input)
{
    VS_POSITION_OUTPUT output;

    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
    }
    output.position = mul(mul(mul(float4(input.position, 1.0f), mtxVertexToBoneWorld),gmtxView), gmtxProjection);

    return (output);
}

struct PS_DEPTH_OUTPUT
{
    float fzPosition : SV_Target;
    float fDepth : SV_Depth;
};

PS_DEPTH_OUTPUT PS_DepthWrite(VS_POSITION_SKINNING_INPUT input)
{
    PS_DEPTH_OUTPUT output;

    output.fzPosition = input.position.z;
    output.fDepth = input.position.z;

    return (output);
}

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
};

VS_STANDARD_OUTPUT VS_Main(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PS_Main(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(samplerState, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(samplerState, input.uv);

	float3 normalW;
	float4 cColor = cAlbedoColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}
	// float4 cIllumination = Lighting(input.positionW, normalW);
    // return (lerp(cColor, cIllumination, 0.5f));
	
    // float4 cColor = float4(0, 0, 0, 1);
    return cColor;
}

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VS_Main_Skinning(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}
	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

// ======== Skybox =========
struct VS_SKYBOX_CUBEMAP_INPUT
{
    float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
    float3 positionL : POSITION;
    float4 position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VS_SkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
    VS_SKYBOX_CUBEMAP_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.positionL = input.position;

    return (output);
}

float4 PS_SkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtCubeTexture.Sample(gssClamp, input.positionL);

    return (cColor);
}


// ======== UI =========
struct VS_UI_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_UI_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_UI_OUTPUT VS_UI(VS_UI_INPUT input)
{
    VS_UI_OUTPUT output;
   
    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return output;
}

VS_UI_OUTPUT VS_UI_SPRITE(VS_UI_INPUT input)
{
    VS_UI_OUTPUT output;
   
    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    //output.uv = mul(float3(input.uv, 1.0f), (float3x3) (gmtxTexture)).xy;
    //if (gmtxTexture._31 == 0.0f)
    //    output.uv += 0.5;
    
    output.uv.x = input.uv.x * width + left;
    output.uv.y = input.uv.y * height + top;
	
    return output;
}

float4 PS_UI(VS_UI_OUTPUT input) : SV_TARGET
{
    float4 textureColor;
	
    // 이 텍스처 좌표 위치에서 샘플러를 사용하여 텍스처에서 픽셀 색상을 샘플링합니다.
    textureColor = gtxtAlbedoTexture.Sample(samplerState, input.uv);

    return textureColor;
}

// ======== BOUNDING_BOX =========

float4 PS_Red(VS_POSITION_OUTPUT input) : SV_TARGET
{
    return (float4(1.0f, 0.0f, 0.0f, 1.0f));
}

struct VS_TERRAIN_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_TERRAIN_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_TERRAIN_OUTPUT VS_Terrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

float4 PS_Terrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    float4 cBaseTexColor = gtxtAlbedoTexture.Sample(samplerState, input.uv);
    float4 cColor = cBaseTexColor;

    return (cColor);
}