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
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
	float3 SizeW   : SIZE;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	// 이 fx를 한번 사용해서 한번에 여러개의 물체를 그리면 그 물체 수 만큼
	// fx 파일이 자동으로 인덱스를 증가 시키면서 해당 그려치는 물체에 대한 ID를 생성한다.
    uint   PrimID  : SV_PrimitiveID; // 고유 물체 ID
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// 정점 쉐이더에서 뭘 하지 않고 그대로 기하 쉐이더로 넘긴다.
	vout.PosW = vin.PosL;
	vout.NormalW = vin.NormalL;
	vout.SizeW   = vin.SizeL;

	return vout;
}

// 선을 받아서 원기둥의 한 면을 구성하는 기하 셰이더
// 이 기하 셰이더를 사용하기 위해서는 적절한 정점 구조가 필요하다.
// 정점 구조는 점의 위치, 크기가 필요한데 이 때 크기는 2가지 정보가
// 필요한데 원기둥의 높이, 원기둥 스택 개수가 이에 해당된다.
// 내가 작성한 코드는 처음에 법선벡터도 필요하고 크기도 슬라이스 정보까지
// 필요한 줄 알아서 정점 구조가 위치, 법선, 크기(float3)으로 짜져있는데
// 막상 짜보니 정보를 이보다 더 적게 필요로 하더라...
// TriangleStream에 출력할 정점을 넣게 되면 삼각형 띠 형태로 출력이 된다.
// 그래서 밑의 코드는 한 면을 완전히 잘 덮어준다.
[maxvertexcount(40)]
void GS(line VertexOut gin[2], uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream) // 완성된 구조를 삼각형 스트림에 담는다.
{
    float Hlen = gin[0].SizeW.x; // 원기둥 높이
    float Cx = gin[0].PosW.x, Cy, Cz = gin[0].PosW.z; // 선의 시작 정점
    float Cx2 = gin[1].PosW.x, Cz2 = gin[1].PosW.z; // 선의 끝 정점
    int StackCnt = gin[0].SizeW.z; // 스택 개수
    float3 N = normalize(float3(Cx, 0, Cz)), N2 = normalize(float3(Cx2, 0, Cz2)); // 두 정점의 법선 벡터
    
    GeoOut gout;
    gout.PrimID = primID;
    
    // 선을 받아서 원기둥의 한면을 구성하는 
    for (int k = 0; k <= StackCnt; k++)
    {
        // 높이 측정
        Cy = -Hlen / 2 + Hlen / StackCnt * k;
        
        // 선의 첫번째 정점 넣고
        gout.PosW = mul(float4(Cx, Cy, Cz, 1.0f), gWorld).xyz;
        gout.NormalW = mul(N, (float3x3) gWorldInvTranspose);
        gout.PosH = mul(float4(Cx, Cy, Cz, 1.0f), gWorldViewProj);
        triStream.Append(gout);
    
        // 선의 두번째 정점 넣고
        gout.PosW = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorld).xyz;
        gout.NormalW = mul(N2, (float3x3) gWorldInvTranspose);
        gout.PosH = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorldViewProj);
        triStream.Append(gout);
    }
}

/*
// 경고표시가 뜨는 기하 셰이더 코드
// 모든 경로에서 gout[]의 값을 읽을 수 없다고 오류를 내뿜는데
// 왜 그런지 이유를 모르겠다.
// 스택 오버플로우에서 찾아봐도 안나온다.
[maxvertexcount(40)]
void GS(line VertexOut gin[2], uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream) // 완성된 구조를 삼각형 스트림에 담는다.
{
    float Hlen = gin[0].SizeW.x;
    float Cx = gin[0].PosW.x, Cy = -Hlen / 2, Cz = gin[0].PosW.z;
    float Cx2 = gin[1].PosW.x, Cz2 = gin[1].PosW.z;
    int StackCnt = gin[0].SizeW.z;
    float3 N = normalize(float3(Cx, 0, Cz)), N2 = normalize(float3(Cx2, 0, Cz2));
    GeoOut gout[4];
    gout[0].PrimID = gout[1].PrimID = gout[2].PrimID = gout[3].PrimID = primID;
    
    // 첫번째 정점 넣고
    gout[0].PosW = mul(float4(Cx, Cy, Cz, 1.0f), gWorld).xyz;
    gout[0].NormalW = mul(N, (float3x3) gWorldInvTranspose);
    gout[0].PosH = mul(float4(Cx, Cy, Cz, 1.0f), gWorldViewProj);
    
    // 두번째 정점 넣고
    gout[1].PosW = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorld).xyz;
    gout[1].NormalW = mul(N2, (float3x3) gWorldInvTranspose);
    gout[1].PosH = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorldViewProj);
    
    for (int k = 1; k <= StackCnt; k++)
    {
        // 높이 측정
        Cy = -Hlen / 2 + Hlen / StackCnt * k;
        
        // 세번째 정점 넣고
        gout[2].PosW = mul(float4(Cx, Cy, Cz, 1.0f), gWorld).xyz;
        gout[2].NormalW = mul(N, (float3x3) gWorldInvTranspose);
        gout[2].PosH = mul(float4(Cx, Cy, Cz, 1.0f), gWorldViewProj);
    
        // 네번째 정점 넣고
        gout[3].PosW = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorld).xyz;
        gout[3].NormalW = mul(N2, (float3x3) gWorldInvTranspose);
        gout[3].PosH = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorldViewProj);
        
        for (int j = 0; j < 4; j++)
        {
            triStream.Append(gout[j]);
        }
		triStream.RestartStrip();
        gout[0] = gout[2];
        gout[1] = gout[3];
    }
}
*/

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
		texColor = gDiffuseMap.Sample(samAnisotropic, float2(0, 0));

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

technique11 Light3Fog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, true)));
    }
}

technique11 Light3AlphaClipFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, true, true)));
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
