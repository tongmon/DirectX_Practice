//***************************************************************************************
// LightHelper.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Structures and functions for lighting calculations.
//***************************************************************************************

struct DirectionalLight // LightHelper.h의 직접광과 대응
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight // LightHelper.h의 점광과 대응
{ 
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att;
	float pad;
};

struct SpotLight // LightHelper.h의 점적광과 대응
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
	float4 Reflect;
};

float Cartoon_Diffuse(float Diff)
{
	float Ans = 0.0f;
	if (Diff <= 0)
		Ans = 0.4;
	else if (0 < Diff && Diff <= 0.5)
		Ans = 0.6;
	else
		Ans = 1.0;
	return Ans;
}

float Cartoon_Specular(float Spec)
{
	float Ans = 0.0f;
	if (Spec <= 0.1)
		Ans = 0.4;
	else if (0.1 < Spec && Spec <= 0.8)
		Ans = 0.5;
	else
		Ans = 0.8;
	return Ans;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(Material mat, DirectionalLight L, 
                             float3 normal, float3 toEye,
					         out float4 ambient,
						     out float4 diffuse,
						     out float4 spec) // 직접광 식 구현
{
	// 초기화
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 표면에서 광원으로의 벡터 (빛 방향 반대 벡터)
	float3 lightVec = -L.Direction;

	// 주변광 계산
	ambient = mat.Ambient * L.Ambient;

	// 빛이 막히지 않고 표면에 도달한다는 가정하에
	// 분산광 항과 반영광 항을 더한다.
	
	float diffuseFactor = dot(lightVec, normal); // 빛 세기 조절을 위한 각도

	// 동적 분기를 피하기 위해 조건문을 펼친다.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal); // 반사 벡터
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w); // 반영광 세기 계산

		diffuseFactor = Cartoon_Diffuse(diffuseFactor);
		specFactor = Cartoon_Specular(specFactor);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse; // 빛 세기를 고려하여 색상 계산
		spec    = specFactor * mat.Specular * L.Specular; // 반영광 세기 고려해서 반영광 색상 계산
	}
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
				   out float4 ambient, out float4 diffuse, out float4 spec) // 점광 식 구현
{
	// 초기화
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 빛 벡터 (표면 점에서 광원으로의 벡터)
	float3 lightVec = L.Position - pos;
		
	// 표면 점과 광원 사이의 거리
	float d = length(lightVec);
	
	// 점광 범위에 벗어났으면 그대로 종료, 아니면 계속 진행
	if( d > L.Range )
		return;
		
	// 벡터 정규화
	lightVec /= d; 
	
	// 주변광 항
	ambient = mat.Ambient * L.Ambient;	

	// 빛이 막히지 않고 표면에 도달한다는 가정하에
	// 분산광 항과 반영광 항을 더한다.

	float diffuseFactor = dot(lightVec, normal); // 빛 세기 조절을 위한 각도

	// 동적 분기를 피하기 위해 조건문을 펼친다.
	// 직접광과 같다.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal); // 반사 벡터
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w); // 반영광 세기 계산

		diffuseFactor = Cartoon_Diffuse(diffuseFactor);
		specFactor = Cartoon_Specular(specFactor);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse; // 빛 세기를 고려하여 색상 계산
		spec    = specFactor * mat.Specular * L.Specular; // 반영광 세기 고려해서 반영광 색상 계산
	}

	// 점광은 거리에 따라서 빛 세기가 달라지니 추가적으로 att를 나누어 준다.
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d*d)); // 1 / (a0 + a1 * d + a2 * d * d)

	diffuse *= att; // 최종 세기 보정
	spec    *= att; // 최종 세기 보정
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
				  out float4 ambient, out float4 diffuse, out float4 spec) // 점적광 식 구현
{
	// 점광과 별 다를 것이 없기에 달라지는 부분만 주석을 달겠다.

	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
	float d = length(lightVec);
	
	// Range test.
	if( d > L.Range )
		return;
		
	// Normalize the light vector.
	lightVec /= d; 
	
	// Ambient term.
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuseFactor = Cartoon_Diffuse(diffuseFactor);
		specFactor = Cartoon_Specular(specFactor);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
	
	// 점적광의 빛 원뿔 모양 범위 제한
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// 점광과 같은 원리로 거리에 따른 세기 조절
	float att = spot / dot(L.Att, float3(1.0f, d, d*d));

	ambient *= spot; // 주변광에는 att를 할 필요없이 원뿔 세기 조절만 가한다.
	diffuse *= att;
	spec    *= att;
}

 
 