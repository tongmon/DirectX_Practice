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

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosL = vin.PosL;

	return vout;
}
 
struct PatchTess
{
	float EdgeTess[3]   : SV_TessFactor;
	float InsideTess[1] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 3> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	float3 centerW = mul(float4(0,0,0, 1.0f), gWorld).xyz; // 구의 중심을 실제 좌표로 이동한 것
	
	float d = distance(centerW, gEyePosW); // 카메라와 거리를 계산하여 테셀레이션 계수 설정

	// 거리가 d1보다 크면(카메라에서 많이 멀면) 테셀레이션 계수를 0으로 하고
	// 거리가 d0보다 작으면(카메라와 많이 가깝다면) 테셀레이션 계수를 64로 한다.
	// 그 사이는 d0, d1을 보간한 값으로 설정한다.
	
	const float d0 = 20.0f;
	const float d1 = 100.0f;
	float tess = 64.0f*saturate( (d1-d)/(d1-d0) );

    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
	
	// 내부 세밀도
    pt.InsideTess[0] = tess;
	
	return pt;
}

struct HullOut
{
	float3 PosL : POSITION;
};

// 제어점 덮개 셰이더는 그대로 통과 시키는 과정이다.
[domain("tri")]
[partitioning("integer")] // fractional_even, fractional_odd, integer
[outputtopology("triangle_cw")]
// 입력 패치에 대해 제어점 덮개 셰이더의 실행 횟수라고 볼 수도 있다.
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 3> p, 
           uint i : SV_OutputControlPointID, // 현재 출력할 제어점의 색인
           uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	
	hout.PosL = p[i].PosL; // 어떤 처리도 안하고 그냥 통과시킨다.
	
	return hout;
}

struct DomainOut
{
	float4 PosH : SV_POSITION;
};

// 영역 셰이더는 테셀레이터가 생성한 각 정점마다 호출된다. 
// 테셀레이션 이후의 정점 셰이더라고 할 수 있다.
[domain("tri")]
DomainOut DS(PatchTess patchTess, 
             float3 uvw : SV_DomainLocation, // 테셀레이션 지역 좌표
             const OutputPatch<HullOut, 3> quad)
{
	DomainOut dout;
	
    float3 p = uvw.x * quad[0].PosL + uvw.y * quad[1].PosL + uvw.z * quad[2].PosL;
    p = normalize(p);
	
	// Hill 예제에서 사용했던 곡면을 도출하는 임의의 함수
	// 정점마다 적용해주어야 테셀레이션 세부도가 높아질수록 언덕형태가 잘 나온다.
	
	dout.PosH = mul(float4(p, 1.0f), gWorldViewProj); // 실제 출력될 좌표
	
	return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f); // 그냥 획득한 픽셀 좌표를 흰색으로 칠한다.
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
