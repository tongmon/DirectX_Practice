//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

// cbuffer --> 상수 버퍼를 의미
cbuffer cbPerObject
{
	// 국소공간에서 실제 무대 좌표로 옮겨주는 S,R,T,시야,투영 이러한 행렬변환이 모두 연산되어 저장되는 결과 행렬
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 Pos   : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

// VS --> 정점 쉐이더
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// 동차 절단 공간으로 변환
	// float4(vin.Pos, 1.0f) --> 4차원 벡터 생성, mul은 HSHL 언어에서 행렬 곱을 해주는 함수
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);
	
	// 정점 색상을 그대로 픽셀 셰이더에 전달
    vout.Color = vin.Color;
    
    return vout;
}

// PS --> 픽셀 쉐이더, VS가 정점단위로 처리가 되었다면 이제는 픽셀단위로 정점 사이가 보간된 값들을 조정한다.
// SV_Target --> 반환값 형식이 렌더 대상 형식과 일치해야 한다.
float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color; // 보간된 색상값 반환
}

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

