//=============================================================================
// Lighting.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and lights geometry.
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float3 gEyePosW;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose; // 역전치행렬, 행렬변환 후에도 알맞은 법선 벡터를 유지시키는 용도로 쓰인다.
	float4x4 gWorldViewProj;
	Material gMaterial;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// 세계 공간으로 변환
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose); // 역전치행렬 곱해서 변환된 올바른 법선벡터를 구해준다.
		
	// 동차 절단 공간으로 변환
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	return vout;
}
  
float4 PS(VertexOut pin) : SV_Target
{
	// 보간으로 인해 법선이 단위벡터가 아닐 수 있으므로 다시 정규화
    pin.NormalW = normalize(pin.NormalW); 

	float3 toEyeW = normalize(gEyePosW - pin.PosW);

	// 성분들의 합이 0인 재질 속성들로 시작
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 각 광원이 기여한 빛을 합한다.
	// 이 예제에서 직접광, 점광, 점적광이 공존하고 있어서 복합적인 빛 작용을 구해야 하기에 더해준다.
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEyeW, A, D, S); // 직접광
	ambient += A;  
	diffuse += D;
	spec    += S;

	ComputePointLight(gMaterial, gPointLight, pin.PosW, pin.NormalW, toEyeW, A, D, S); // 점광
	ambient += A;
	diffuse += D;
	spec    += S;

	ComputeSpotLight(gMaterial, gSpotLight, pin.PosW, pin.NormalW, toEyeW, A, D, S); // 점적광
	ambient += A;
	diffuse += D;
	spec    += S;
	   
	float4 litColor = ambient + diffuse + spec; // 직접광, 점광, 점적광이 더해진 최종 픽셀의 색상

	// 분산광 재질의 알파와 텍스처의 알파의 곱을 전체적인 알파 값으로 사용한다.
	litColor.a = gMaterial.Diffuse.a;

    return litColor;
}

technique11 LightTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
