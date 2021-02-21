//=============================================================================
// Sky.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// 하늘 돔 렌더링에 쓰이는 효과
//=============================================================================

// 자연스러운 하늘을 표현하기 위해 조회 벡터를 타원으로 구성한다.
// 또 플레이어가 하늘에 도달할 수 없게 하늘의 입방체 맵 중심은
// 카메라(시점)의 중심으로 설정한다.
// 그러면 플레이어가 계속 어디를 가도 하늘은 계속 플레이어와 적정 거리를 유지할 것이다.

cbuffer cbPerFrame
{
	float4x4 gWorldViewProj;
};
 
// 입방체 맵 형태의 TextureCube
TextureCube gCubeMap;

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// z/w = 1 이 되도록 (하늘 돔이 항상 먼 평면에 있도록) z = w로 설정한다.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj).xyww;
	
	// 국소 정점 위치를 입방체 맵 조회 벡터로 사용한다.
	vout.PosL = vin.PosL;
	
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return gCubeMap.Sample(samTriLinearSam, pin.PosL);
}

RasterizerState NoCull
{
    CullMode = None;
};

DepthStencilState LessEqualDSS
{
	// 깊이 함수를 그냥 < 가 아니라 <=로 해야 한다. 
	// 그렇게 하지 않으면, 깊이 버퍼를 1로 지웠다고 할 때 z = 1 (NDC)의
	// 정규화된 깊이 값들이 깊이 판정에 실패한다.
    DepthFunc = LESS_EQUAL;
};

technique11 SkyTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
        
        SetRasterizerState(NoCull);
        SetDepthStencilState(LessEqualDSS, 0);
    }
}
