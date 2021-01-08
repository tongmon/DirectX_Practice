//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform; // 텍스쳐 매핑
	Material gMaterial;
}; 

// 비수치 값은 cbuffer에 추가할 수 없다.
Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC; // 비등방 필터링 옵션
	MaxAnisotropy = 4;

	// 좌표 지정모드
	// WRAP --> 타일링, 0 ~ 1이외의 텍스쳐 좌표는 타일링 기법을 적용
	// BORDER --> 0 ~ 1 이외의 텍스쳐 좌표는 프로그래머가 지정한 색상 반환
	// CLAMP --> 한정 모드, 0 ~ 1 이외의 텍스쳐 좌표는 가장 가까운 텍스쳐 점의 색상을 가짐
	// MIRROR --> 거울 모드, 타일링과 비슷한데 거울 같이 방향이 반전
	AddressU = WRAP;
	AddressV = WRAP;

	// BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f); // BORDER인 경우 지정 색상
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD; // 텍스쳐 좌표
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD; // 텍스쳐 좌표
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// 세계 공간으로 변환
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	// 동차 절단 공간으로 변환
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// 정점 특성들을 출력한다. (이후 삼각형 표면을 따라 보간된다.)
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}

// gUseTexure에 따라 텍스쳐 적용 여부 판단
float4 PS(VertexOut pin, uniform int gLightCount, uniform bool gUseTexure) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye); 

	// Normalize.
	toEye /= distToEye;
	
    // 기본 텍스처 색상은 곱셈의 항등원
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// 텍스쳐에서 표본을 추출한다.
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );
	}
	 
	//
	// Lighting.
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{  
		// Start with a sum of zero. 
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		[unroll]
		for(int i = 0; i < gLightCount; ++i)
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		// 변조 후 가산, 이러한 방법이 일반적
		litColor = texColor * (ambient + diffuse) + spec;
	}

	// 일반적으로 분산광 재질의 알파 성분을 최종 알파 값으로 사용한다.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

technique11 Light1
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, false) ) );
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, false) ) );
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false) ) );
    }
}

technique11 Light0Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true) ) );
    }
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, true) ) );
    }
}

technique11 Light2Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true) ) );
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true) ) );
    }
}
