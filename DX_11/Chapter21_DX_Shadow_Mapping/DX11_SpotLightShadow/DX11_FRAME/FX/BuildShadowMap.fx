//=============================================================================
// BuildShadowMap.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Effect used to build the shadow map.
//
// 그림자 맵에 깊이를 그릴때,
// 그림자 맵이 눈이 보는 것과 같은
// 지형을 기록하도록
// 눈에서 렌더링 할 때
// 동일한 방식으로
// 지형을 테셀레이션해야 한다.
// 즉 눈에서 거리에 따라 세분화 된 도형들에 따라 그림자도 세분화되어야 하니까
// 이 세분화 상태를 획득하고 이를 적용한 상황에서 그림자 맵을 생성해야한다는 말이다.
//=============================================================================

#include "LightHelper.fx"

cbuffer cbPerFrame
{
    SpotLight gSpotLight;
	
	float3 gEyePosW;
	
	float gHeightScale;
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMinTessFactor;
	float gMaxTessFactor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gViewProj;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
}; 

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;
Texture2D gNormalMap;
 
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
	float2 Tex  : TEXCOORD;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// 여기서 gWorldViewProj 이 녀석이 곱해진 View, Proj는
	// 광원 중심의 값이다.
	// Proj는 직교 투영이며 View는 카메라 위치가 아닌 광원 중심 좌표계에 따른 행렬이다.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.Tex  = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}

struct TessVertexOut
{
	float3 PosW       : POSITION;
	float3 NormalW    : NORMAL;
	float2 Tex        : TEXCOORD;
	float  TessFactor : TESS;
};

TessVertexOut TessVS(VertexIn vin)
{
	TessVertexOut vout;

	vout.PosW     = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW  = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.Tex      = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	float d = distance(vout.PosW, gEyePosW);

	// Normalized tessellation factor. 
	// The tessellation is 
	//   0 if d >= gMinTessDistance and
	//   1 if d <= gMaxTessDistance.  
	float tess = saturate( (gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance) );
	
	// Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
	vout.TessFactor = gMinTessFactor + tess*(gMaxTessFactor-gMinTessFactor);

	return vout;
}

struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess  : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<TessVertexOut,3> patch, 
                  uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.EdgeTess[0] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[2].TessFactor + patch[0].TessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
	pt.InsideTess  = pt.EdgeTess[0];
	
	return pt;
}

struct HullOut
{
	float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
	float2 Tex      : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<TessVertexOut,3> p, 
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	
	// Pass through shader.
	hout.PosW     = p[i].PosW;
	hout.NormalW  = p[i].NormalW;
	hout.Tex      = p[i].Tex;
	
	return hout;
}

struct DomainOut
{
	float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
	float2 Tex      : TEXCOORD;
};

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("tri")]
DomainOut DS(PatchTess patchTess, 
             float3 bary : SV_DomainLocation, 
             const OutputPatch<HullOut,3> tri)
{
	DomainOut dout;
	
	// Interpolate patch attributes to generated vertices.
	dout.PosW     = bary.x*tri[0].PosW     + bary.y*tri[1].PosW     + bary.z*tri[2].PosW;
	dout.NormalW  = bary.x*tri[0].NormalW  + bary.y*tri[1].NormalW  + bary.z*tri[2].NormalW;
	dout.Tex      = bary.x*tri[0].Tex      + bary.y*tri[1].Tex      + bary.z*tri[2].Tex;
	
	// Interpolating normal can unnormalize it, so normalize it.
	dout.NormalW = normalize(dout.NormalW);
	
	//
	// Displacement mapping.
	//
	
	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	const float MipInterval = 20.0f;
	float mipLevel = clamp( (distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);
	
	// Sample height map (stored in alpha channel).
	float h = gNormalMap.SampleLevel(samLinear, dout.Tex, mipLevel).a;
	
	// Offset vertex along normal.
	dout.PosW += (gHeightScale*(h-1.0))*dout.NormalW;
	
	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
	
	return dout;
}

/*
LightHelper.fx에 따로 만들어둔 함수이고
해당 픽셀이 점적광을 받는 픽셀인지 아닌지 판단해준다.
점적광은 spot 값에 따라 중심에서 멀어질 수록 빛이 약해지는 효과를 구현하는데
이 값이 많이 작다고 판단되면 clip을 진행할 수 있도록 결과를 도출해주는 함수이다.
여기선 0.01이 알맞은 값이라고 판된되어서 이렇게 했는데 더 큰값으로 하면 더 많은 픽셀이
clip될 것이다.
bool IsUnSpotPixel(SpotLight L, float3 pos)
{
    bool result = false;
	
	// The vector from the surface to the light.
    float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
    float d = length(lightVec);
	
    if (d > L.Range)
        result = true;
	
	// Normalize the light vector.
    lightVec /= d;
	
	// Scale by spotlight factor and attenuate.
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
	
    if (spot <= 0.01f)
        result = true;
	
    return result;
}
*/

// 이 픽셀 셰이더는 알파 값 기반 투명 패턴을 가진 기하구조에만
// 쓰인다. 깊이 패스의 경우 텍스쳐를 추출할 필요가 없는
// 기하구조에 대해서는 NULL 픽셀 셰이더를 사용해도 된다.
void PS(VertexOut pin)
{
    // 위에서 언급한 IsUnSpotPixel 함수를 통해 해당 픽셀이 점적광 빛을 받는지 아닌지
	// 판단해서 clip 픽셀을 판단한다.
	bool isClip = IsUnSpotPixel(gSpotLight, pin.PosW);
    if (isClip)
        clip(-0.1f);
}

// This is only used for alpha cut out geometry, so that shadows 
// show up correctly.  Geometry that does not need to sample a
// texture can use a NULL pixel shader for depth pass.
void TessPS(DomainOut pin)
{
	float4 diffuse = gDiffuseMap.Sample(samLinear, pin.Tex);

	// Don't write transparent pixels to the shadow map.
	clip(diffuse.a - 0.15f);
}

RasterizerState Depth
{
	// [From MSDN]
	// 현재 출력 병합기 단계에 묶인 깊이 버퍼의 형식이 UFORM이거나
	// 출력 병합기 단계에 아무런 깊이 버퍼도 묶여 있지 않다면,
	// 편향치는 다음과 같이 계산된다.

	// Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
	// 여기서 r은 0보다 큰 최솟값이다.
	// [/End MSDN]

	// 24-bit 깊이 버퍼의 경우, r = 1 / 2^24이다.
	//
	// 예: DepthBias = 100000 ==> Actual DepthBias = 100000/2^24 = .006

	// 장면에 잘 맞는 값들은 실험을 거쳐서 구해야 한다.
	DepthBias = 100000;
    DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 1.0f;
};

technique11 BuildShadowMapTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_5_0, PS()) );

		SetRasterizerState(Depth);
    }
}

technique11 BuildShadowMapAlphaClipTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 TessBuildShadowMapTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, TessVS() ) );
		SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( NULL );

		SetRasterizerState(Depth);
    }
}

technique11 TessBuildShadowMapAlphaClipTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, TessVS() ) );
		SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, TessPS() ) );
    }
}