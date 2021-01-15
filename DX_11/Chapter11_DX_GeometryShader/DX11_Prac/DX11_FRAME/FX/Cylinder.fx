//***************************************************************************************
// TreeSprite.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// 기하 쉐이더를 이용해서 점 스프라이트를 y축에 정렬된 카메라 방향 빌보드 사각형으로 확장한다.
//***************************************************************************************

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
	float4x4 gTexTransform;
	Material gMaterial;
};

// 비수치 값들은 cbuffer에 추가할 수 없음
Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float3 SizeL   : SIZE;
};

struct VertexOut
{
	float3 CenterW : POSITION;
	float2 SizeW   : SIZE;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex     : TEXCOORD;
	// 이 fx를 한번 사용해서 한번에 여러개의 물체를 그리면 그 물체 수 만큼
	// fx 파일이 자동으로 인덱스를 증가 시키면서 해당 그려치는 물체에 대한 ID를 생성한다.
    uint   PrimID  : SV_PrimitiveID; // 고유 물체 ID
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// 정점 쉐이더에서 뭘 하지 않고 그대로 기하 쉐이더로 넘긴다.
	vout.CenterW = vin.PosW;
	vout.SizeW   = vin.SizeW;

	return vout;
}
 
// 각 점을 사각형(정점 4개)으로 확장하므로, 기하 쉐이더 실행 당
// 최대 출력 정점 개수는 4이다.
[maxvertexcount(4)]
void GS(point VertexOut gin[1], // 점을 하나 넣고
        uint primID : SV_PrimitiveID, 
        inout TriangleStream<GeoOut> triStream) // 완성된 구조를 삼각형 스트림에 담는다.
{	
	//
	// 빌보드가 y축에 정렬되어서 시점을 향하도록 하는,
	// 세계 공간을 기준으로 한 빌보드의 국소 좌표계를 계산한다.
	//

	float3 up = float3(0.0f, 1.0f, 0.0f); // y 축에 수직, 나무는 위로 자라니까.
	float3 look = gEyePosW - gin[0].CenterW; // 나무 중심에서 카메라까지 방향의 벡터 획득
	look.y = 0.0f; // y축 정렬이라 했으니 y는 0으로 고정한다.
	look = normalize(look); // 정규화하여 크기 1로 만든다.
	// 외적을 통해 나무 중심에서 시점까지 벡터와 y축 모두에게 수직인 오른쪽 벡터를 획득한다.
	float3 right = cross(up, look);

	//
	// 사각형을 이루는 삼각형 띠 정점들의 세계 공간 위치를 계산한다.
	//
	float halfWidth  = 0.5f*gin[0].SizeW.x; // 너비의 절반
	float halfHeight = 0.5f*gin[0].SizeW.y; // 높이의 절반
	
	/*
	v[3] ------- v[1]
	 |             |
	 |             |
	 |   CenterW   |
	 |             |
	 |             |
	v[2] ------- v[0]
	이러한 사각형이 카메라를 정면으로 응시하게 만드는 것이다.
	*/
	float4 v[4]; 
	v[0] = float4(gin[0].CenterW + halfWidth*right - halfHeight*up, 1.0f);
	v[1] = float4(gin[0].CenterW + halfWidth*right + halfHeight*up, 1.0f);
	v[2] = float4(gin[0].CenterW - halfWidth*right - halfHeight*up, 1.0f);
	v[3] = float4(gin[0].CenterW - halfWidth*right + halfHeight*up, 1.0f);

	//
	// 사각형 정점들을 동차 절단 공간으로 변환하고
	// 하나의 삼각형을 띠로서 출력한다.
	//
	GeoOut gout;
	[unroll]
	for(int i = 0; i < 4; ++i)
	{
		gout.PosH     = mul(v[i], gViewProj); // 시점, 투영 행렬을 곱해준다.
		gout.PosW     = v[i].xyz;
		gout.NormalW  = look;
		gout.Tex      = gTexC[i];
		gout.PrimID   = primID;
		
		triStream.Append(gout);
	}
}

float4 PS(GeoOut pin, uniform int gLightCount, uniform bool gUseTexure, uniform bool gAlphaClip, uniform bool gFogEnabled) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
   
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// 텍스쳐에서 표본 추출
		float3 uvw = float3(pin.Tex, pin.PrimID%4); // PrimID마다 각기 다른 나무 텍스쳐 사용
		// uvw --> (텍스쳐 x, 텍스쳐 y, 텍스쳐 배열 인덱스 번호)
		texColor = gTreeMapArray.Sample( samLinear, uvw );

		if(gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.05.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.05f);
		}
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

		// Modulate with late add.
		litColor = texColor*(ambient + diffuse) + spec;
	}

	//
	// Fogging
	//

	if( gFogEnabled )
	{
		float fogLerp = saturate( (distToEye - gFogStart) / gFogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

//---------------------------------------------------------------------------------------
// Techniques-- 필요하면 알아서 알맞게 추가, 변형하면 된다.
//---------------------------------------------------------------------------------------
technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, false) ) );
    }
}

technique11 Light3TexAlphaClip
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, false) ) );
    }
}
            
technique11 Light3TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, true) ) );
    }
}
