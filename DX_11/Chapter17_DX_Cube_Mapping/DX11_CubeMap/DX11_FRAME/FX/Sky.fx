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
	
	// z / w = 1 이 되도록 (하늘 돔이 항상 먼 평면에 있도록) z = w로 설정한다.
	// z는 근간을 나타내는 척도다. 이게 무슨 말이냐면
	// gWorldViewProj 이 행렬을 거쳤다는 것은 중심좌표계 기준이 카메라 좌표계 기준이 되고
	// 그 후에 투영을 거쳐 w로 나누게 되면 정규화된 NDC 좌표계로 변화된다.
	// 카메라 좌표계는 z 축이 바라보는 눈에서 레이져를 쏘게되는 축인데(화면의 중심, CrossHair)
	// z 축 값이 멀어질 수록 해당 물체는 멀게 보일 것이다.
	// 근데 정규화된 NDC 좌표계에서 z의 범위는 0 ~ 1 이고 제일 먼 경우가 1이다.
	// 따라서 정규화된 NDC 좌표계에서 하늘 텍스쳐를 닿을 수 없을 정도로 먼 느낌을
	// 플레이어에게 주기 위해 z를 w로 설정해 NDC 좌표계에서 실질적인 z값을 1로 만들어주는 것이다. 
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj).xyww;
	
	// 국소 정점 위치를 입방체 맵 조회 벡터로 사용한다.
	// 하늘 구에 정점 중에 하나를 잡고 국소 공간의 원점에서 그 정점까지의 벡터가
	// 가리키는 곳의 텍스쳐가 출력될 것이다.
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
