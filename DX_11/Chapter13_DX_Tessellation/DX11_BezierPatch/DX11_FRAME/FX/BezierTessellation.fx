 
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

// Nonnumeric values cannot be added to a cbuffer.
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
};

struct VertexOut
{
	float3 PosL    : POSITION;
};

// 테셀레이션이 활성화 되면 정점 셰이더는 주로 제어점 별로
// 애니메이션이나 물리를 계산한다.
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosL = vin.PosL;

	return vout;
}
 
struct PatchTess
{
	float EdgeTess[4]   : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 16> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	// 테셀레이션 계수 설정

	pt.EdgeTess[0] = 25;
	pt.EdgeTess[1] = 25;
	pt.EdgeTess[2] = 25;
	pt.EdgeTess[3] = 25;
	
	pt.InsideTess[0] = 25;
	pt.InsideTess[1] = 25;

	return pt;
}

struct HullOut
{
	float3 PosL : POSITION;
};

// This Hull Shader part is commonly used for a coordinate basis change, 
// for example changing from a quad to a Bezier bi-cubic.
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 16> p, 
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	
	hout.PosL = p[i].PosL;
	
	return hout;
}

struct DomainOut
{
	float4 PosH : SV_POSITION;
};

// 삼차 베지에 곡선에 대한 베른슈타인 기저 함수들을
// float4에 순서대로 점이 P0, P1, P2, P3인 경우로
// 삽입하였다. 모두 더하면 t에 따른 곡선식이 된다.
// t는 보간 비율로 0 ~ 1 사이값이다.
float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;

    return float4( invT * invT * invT,
                   3.0f * t * invT * invT,
                   3.0f * t * t * invT,
                   t * t * t );
}

// 베지어 곡선 함수에 대한 도함수
float4 dBernsteinBasis(float t)
{
    float invT = 1.0f - t;

    return float4(-3 * invT * invT,
                   3 * invT * invT - 6 * t * invT,
                   6 * t * invT - 3 * t * t,
                   3 * t * t);
}

// 삼차 베지에 곡면을 점 16개로 구성하는 함수이다.
// 일단 각 행에서 점 4개로 베지어 곡선을 만든다.
// 그러면 곡선이 행으로 4개가 생길텐데 이것을 이번엔 열로보고
// 생기는 각 점 4개로 다시 베지어 곡선을 만든다.
// 그러면 행으로도 곡선, 열로도 곡선이 생기면서 곡면이 만들어진다.
// 이를 식으로 밑과 같이 나타낸 것이다.
// 여기서 xyzw는 차원을 의미하는 것이 아닌 점이 P0, P1 이런거 구분하기 위한 것임.
float3 CubicBezierSum(const OutputPatch<HullOut, 16> bezpatch, float4 basisU, float4 basisV)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    sum  = basisV.x * (basisU.x*bezpatch[0].PosL  + basisU.y*bezpatch[1].PosL  + basisU.z*bezpatch[2].PosL  + basisU.w*bezpatch[3].PosL );
    sum += basisV.y * (basisU.x*bezpatch[4].PosL  + basisU.y*bezpatch[5].PosL  + basisU.z*bezpatch[6].PosL  + basisU.w*bezpatch[7].PosL );
    sum += basisV.z * (basisU.x*bezpatch[8].PosL  + basisU.y*bezpatch[9].PosL  + basisU.z*bezpatch[10].PosL + basisU.w*bezpatch[11].PosL);
    sum += basisV.w * (basisU.x*bezpatch[12].PosL + basisU.y*bezpatch[13].PosL + basisU.z*bezpatch[14].PosL + basisU.w*bezpatch[15].PosL);

    return sum;
}

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("quad")]
DomainOut DS(PatchTess patchTess, 
             float2 uv : SV_DomainLocation, 
             const OutputPatch<HullOut, 16> bezPatch)
{
	DomainOut dout;
	
	// uv에 따라 곡면의 점 하나가 결정된다.
	float4 basisU = BernsteinBasis(uv.x);
	float4 basisV = BernsteinBasis(uv.y);

	float3 p  = CubicBezierSum(bezPatch, basisU, basisV); // p(u,v)
	
	/*
	// p(u,v)의 편도 함수들 구하는 방식
	float4 dbasisU = dBernsteinBasis(uv.x);
	float4 dbasisV = dBernsteinBasis(uv.y);
	
	float3 dpdu = CubicBezierSum(bezPatch, dbasisU, basisV);
	float3 dpdv = CubicBezierSum(bezPatch, basisV, dbasisU);
	*/
	
	dout.PosH = mul(float4(p, 1.0f), gWorldViewProj); // 실제좌표로 옮기기
	
	return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

technique11 Tess
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
