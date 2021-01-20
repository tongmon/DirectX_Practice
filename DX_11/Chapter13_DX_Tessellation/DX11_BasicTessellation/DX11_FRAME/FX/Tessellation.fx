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
	float EdgeTess[4]   : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	// 세계 공간에서의 패치 중점을 구한다.
	// 사각형의 중점이니 4를 나누어준다. (0.25f)
	float3 centerL = 0.25f*(patch[0].PosL + patch[1].PosL + patch[2].PosL + patch[3].PosL);
	float3 centerW = mul(float4(centerL, 1.0f), gWorld).xyz; // 중심을 실제 좌표로 이동한 것
	
	float d = distance(centerW, gEyePosW); // 카메라와 거리를 계산하여 테셀레이션 계수 설정

	// 거리가 d1보다 크면(카메라에서 많이 멀면) 테셀레이션 계수를 0으로 하고
	// 거리가 d0보다 작으면(카메라와 많이 가깝다면) 테셀레이션 계수를 64로 한다.
	// 그 사이는 d0, d1을 보간한 값으로 설정한다.
	
	const float d0 = 20.0f;
	const float d1 = 100.0f;
	float tess = 64.0f*saturate( (d1-d)/(d1-d0) );

	// 모든 테셀레이션 개수를 동일하게 설정해서 패치를 균등하게 세분한다.
	// 각 변(모서리) 마다의 세밀도라고 보면 된다.
	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;
	pt.EdgeTess[3] = tess;
	
	// 내부 세밀도
	pt.InsideTess[0] = tess;
	pt.InsideTess[1] = tess;
	
	return pt;
}

struct HullOut
{
	float3 PosL : POSITION;
};

// 제어점 덮개 셰이더는 그대로 통과 시키는 과정이다.
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
// 그대로 통과이기 때문에 입력과 출력 제어점의 개수가 4개로 같다.
// 입력 패치에 대해 제어점 덮개 셰이더의 실행 횟수라고 볼 수도 있다.
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 4> p, 
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
[domain("quad")]
DomainOut DS(PatchTess patchTess, 
             float2 uv : SV_DomainLocation, // 테셀레이션 지역 좌표
             const OutputPatch<HullOut, 4> quad)
{
	DomainOut dout;
	
	// 겹선형 보간으로 실제 정점 위치 유도
	// lerp(x,y,s) ---> x + s(y - x)를 의미하고 
	// uv는 테셀레이션된 정점 위치, 즉 테셀레이션 되어있는 좌표계에서
	// 어디에 위치해 있는지를 나타낸다.
	// uv 값은 0 ~ 1사이
	float3 v1 = lerp(quad[0].PosL, quad[1].PosL, uv.x); 
	float3 v2 = lerp(quad[2].PosL, quad[3].PosL, uv.x); 
	float3 p  = lerp(v1, v2, uv.y); 
	
	// Hill 예제에서 사용했던 곡면을 도출하는 임의의 함수
	// 정점마다 적용해주어야 테셀레이션 세부도가 높아질수록 언덕형태가 잘 나온다.
	p.y = 0.3f*( p.z*sin(p.x) + p.x*cos(p.z) );
	
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
