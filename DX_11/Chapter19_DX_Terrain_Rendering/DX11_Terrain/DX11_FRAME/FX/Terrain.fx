 
#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
	
	// 거리가 최소이면 테셀레이션 계수는 최대이다.
	// 거리가 최대이면 테셀레이션 계수는 최소이다.
	float gMinDist;
	float gMaxDist;

	// 2의 제곱수 형태의 테셀레이션 계수를 위한 지수. 테셀레이션 계수의
	// 범위는 2 ^ MinTess, 2 ^ MaxTess까지이고 MinTess는 적어도 0 이상
	// MaxTess는 많아야 6이하여야 한다.
	float gMinTess;
	float gMaxTess;
	
	// 정규화된 uv 공간 [0, 1] 에서의 높이 값들 사이의 간격.
	float gTexelCellSpaceU;
	float gTexelCellSpaceV;
	// 세계 공간에서 낱칸들 사이의 간격.
	float gWorldCellSpace;
	float2 gTexScale = 50.0f;
	
	// 세계 공간에서 절두체 평면의 평면 계수 6면 저장
	float4 gWorldFrustumPlanes[6];
};

cbuffer cbPerObject
{
	// Terrain coordinate specified directly 
	// at center of world space.
	
	float4x4 gViewProj;
	Material gMaterial;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2DArray gLayerMapArray;
Texture2D gBlendMap;
Texture2D gHeightMap;

SamplerState samLinear // 지형 텍스쳐에 대한 필터
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samHeightmap // 높이맵에 대한 텍스쳐 필터
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VertexIn
{
	float3 PosL     : POSITION;
	float2 Tex      : TEXCOORD0;
	float2 BoundsY  : TEXCOORD1;
};

struct VertexOut
{
	float3 PosW     : POSITION;
	float2 Tex      : TEXCOORD0;
	float2 BoundsY  : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// 이 예제에서 입력된 지형 패치 정점들은 이미 세계 공간에 있다.
	// 이미 0,0이 중심이고 가로, 세로 크기가 2049x2049이며 0.5 간격으로 격자가 생성된다.
	// 따라서 월드 변환을 곱할 필요가 없다.
	vout.PosW = vin.PosL;

	// 패치 모퉁이(제어점들) 정점들을 적절한 높이로 조정한다.
	// 이렇게 하면 이후 시점에서 패치까지의 거리 계산이 좀 더 명확해진다.
	vout.PosW.y = gHeightMap.SampleLevel( samHeightmap, vin.Tex, 0 ).r;

	// 정점 특성들을 다음 단계로 출력한다.
	vout.Tex      = vin.Tex;
	vout.BoundsY  = vin.BoundsY;
	
	return vout;
}
 
float CalcTessFactor(float3 p)
{
	float d = distance(p, gEyePosW);

	// 여기서는 변위 매핑시에 사용했던 함수와 달리
	// 세부수준이 2의 배수로 적용되어 거리에 따른 차이가 좀 더 잘보인다.
	// 그니까 2의 s 승에서 s값이 보간되면서 TessFactor가 2 ^ s로 결정된다.
	
	float s = saturate( (d - gMinDist) / (gMaxDist - gMinDist) );
	
	return pow(2, (lerp(gMaxTess, gMinTess, s)) );
}

// 상자 전체가 평면의 뒤쪽(음의 반공간)에 있다면 true를 돌려준다.
// 평면 계수에서 ax + by + cz + w = 0 에서 (a, b, c)는 평면의 법선 벡터를 의미한다.
// 절두체 평면들의 법선은 항상 공간 내부를 가리키고 있다.
// 따라서 절두체 평면들의 앞면은 안쪽을 항상 바라보고 있다.
bool AabbBehindPlaneTest(float3 center, float3 extents, float4 plane)
{
    float3 n = abs(plane.xyz); // |n.x|, |n.y|, |n.z|
	
	// 반지름은 항상 양수
	// extents는 x,y,z 축 방향으로의 반지름이고 이는 합쳐져 대각선으로의 사각형 반지름이 된다.
	// 이를 벡터 n에 투영하는 연산은 밑과 같다. (n이 크기 1이여서 투영가능)
	// 결과적으로 밑 까지의 연산은
	// |x 반지름을 n에 투영한 길이| + |y 반지름을 n에 투영한 길이| + |z 반지름을 n에 투영한 길이|
	// 즉 AABB의 반지름을 벡터 n에 투영한 길이를 나타낸다.
	// 절댓값은 n 벡터를 구할 때 이미 붙였고 반지름은 이미 양수라 또 절댓값화를 할 필요가 없다.
	// OBB 충돌에서 배웠었지만 다시 말하면
	// 한 방에 r = dot(extents, plane.xyz)를 쓰지 않는 이유가 있다.
	// 이유는 법선 벡터로 투영을 했을 때 가장 긴 길이를 만드는 모퉁이(대각선 반지름) 벡터로 r을 만들어야 하는데
	// 그냥 주어진 모퉁이로 냅다 평면의 법선 벡터에 투영시키면 이게 가장 긴 길이를 만들지 않을 수 있기에
	// dot(extents, abs(plane.xyz)) 이 계산을 하는 것이다.
	float r = dot(extents, n);
	
	// 평면 중점과의 부호 있는 거리.
	// 평면과 점 사이의 거리를 구하는 공식인데 왜 평면의 법선의 크기로 나누지 않느냐면
	// 이미 이 함수에 들어오는 평면들의 법선 벡터는 정규화가 되어서 크기가 1이기 때문이다.
	// 거리는 음수로 나올 수도 있게 절댓값을 씌우지 않음
	float s = dot( float4(center, 1.0f), plane );
	
	// 상자의 중점이 평면 뒤쪽으로 거리 r이상 떨어져 있으면
	// (평면 뒤쪽이므로 s는 음수) 상자 전체가 평면의 음의 반공간에 있는 것
	return (s + r) < 0.0f;
}

// 상자가 절두체 바깥이면 true를 돌려준다.
bool AabbOutsideFrustumTest(float3 center, float3 extents, float4 frustumPlanes[6])
{
	for(int i = 0; i < 6; ++i)
	{
		// 상자 전체가 절두체의 적어도 한 평면의 뒤쪽에 있다면
		// 상자는 절두체 바깥에 있는 것이다.
		if( AabbBehindPlaneTest(center, extents, frustumPlanes[i]) )
		{
			return true;
		}
	}
	
	return false;
}

struct PatchTess
{
	float EdgeTess[4]   : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	//
	// 절두체 선별
	//
	
	// 패치의 BoundY를 첫 번째 제어점에 저장해 두었고 이를 가져와 사용.
	float minY = patch[0].BoundsY.x;
	float maxY = patch[0].BoundsY.y;
	
	// 축 정렬 경계상자를 만든다.
	/*
	0 --- 1
	|	  |
	|	  |
	2 --- 3
	1이 x,z가 4개 정점 중에 가장 크고 2가 x,z가 가장 작다.
	*/
	float3 vMin = float3(patch[2].PosW.x, minY, patch[2].PosW.z);
	float3 vMax = float3(patch[1].PosW.x, maxY, patch[1].PosW.z);
	
	// AABB의 중점, AABB의 반지름
	float3 boxCenter  = 0.5f*(vMin + vMax);
	float3 boxExtents = 0.5f*(vMax - vMin);
	if( AabbOutsideFrustumTest(boxCenter, boxExtents, gWorldFrustumPlanes) )
	{
		// 절두체를 벗어난 패치는 테셀레이션 계수를 0으로 만들어 절단한다.
		pt.EdgeTess[0] = 0.0f;
		pt.EdgeTess[1] = 0.0f;
		pt.EdgeTess[2] = 0.0f;
		pt.EdgeTess[3] = 0.0f;
		
		pt.InsideTess[0] = 0.0f;
		pt.InsideTess[1] = 0.0f;
		
		return pt;
	}
	//
	// 거리에 기초해서 테셀레이션 계수를 계산한다.
	//
	else 
	{
		// 테셀레이션 계수들을 변의 속성에 근거해서 계산하는 것이 중요하다.
		// 그래야 여러 패치가 공유하는 변의 테셀레이션 계수가 그러한 모든 패치에서 동일해진다.
		// 그렇지 않으면 테셀레이션 된 메시에 틈이 생긴다.
		
		// 각 변의 중점과 패치 자체의 중점을 계산한다.
		float3 e0 = 0.5f*(patch[0].PosW + patch[2].PosW);
		float3 e1 = 0.5f*(patch[0].PosW + patch[1].PosW);
		float3 e2 = 0.5f*(patch[1].PosW + patch[3].PosW);
		float3 e3 = 0.5f*(patch[2].PosW + patch[3].PosW);
		float3  c = 0.25f*(patch[0].PosW + patch[1].PosW + patch[2].PosW + patch[3].PosW); // 사각형 중심
		
		// 카메라와 변 중점 거리에 따라 테셀레이션 계수 설정
		pt.EdgeTess[0] = CalcTessFactor(e0);
		pt.EdgeTess[1] = CalcTessFactor(e1);
		pt.EdgeTess[2] = CalcTessFactor(e2);
		pt.EdgeTess[3] = CalcTessFactor(e3);
		// 사각형 중심과 카메라 거리에 따라 테셀레이션 계수 설정
		pt.InsideTess[0] = CalcTessFactor(c);
		pt.InsideTess[1] = pt.InsideTess[0];
	
		return pt;
	}
}

struct HullOut
{
	float3 PosW     : POSITION;
	float2 Tex      : TEXCOORD0;
};

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 4> p, 
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	
	// Pass through shader.
	hout.PosW     = p[i].PosW;
	hout.Tex      = p[i].Tex;
	
	return hout;
}

struct DomainOut
{
	float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
	float2 Tex      : TEXCOORD0;
	float2 TiledTex : TEXCOORD1;
};

// 테셀레이션에서 영역 쉐이더는 테셀레이션 된 정점들의 정점 쉐이더 역할을 대신한다.
[domain("quad")]
DomainOut DS(PatchTess patchTess, 
             float2 uv : SV_DomainLocation, 
             const OutputPatch<HullOut, 4> quad)
{
	DomainOut dout;
	
	// 사각형 패치의 테셀레이션 진행
	
	// 가로에 대한 실제 정점 정보를 보간을 통해 끌어낸다.
	dout.PosW = lerp(
		lerp(quad[0].PosW, quad[1].PosW, uv.x),
		lerp(quad[2].PosW, quad[3].PosW, uv.x),
		uv.y); 
	
	// 텍스쳐 좌표도 보간을 통해 얻는다.
	dout.Tex = lerp(
		lerp(quad[0].Tex, quad[1].Tex, uv.x),
		lerp(quad[2].Tex, quad[3].Tex, uv.x),
		uv.y); 
		
	// 지형에 텍스쳐 계층들을 타일링한다.
    dout.TiledTex = dout.Tex * gTexScale;
	
	// 보간된 uv를 통해 획득된 Tex 좌표를 가지고 변위 매핑.
	dout.PosW.y = gHeightMap.SampleLevel( samHeightmap, dout.Tex, 0 ).r;
	
	// 영역 쉐이더에서 유한차분법을 이용해서 법선을 계산해 보았지만
	// fractional_even의 경우 정점들이 끊임없이 움직이기 때문에 법선의
	// 변화에 따라 빛이 가물거리는 현상이 두드러졌다. 그래서 결국은 법선
	// 계산을 픽셀 셰이더로 옮겼다고 한다.
	
	// 동차 절단 공간으로 투영
	dout.PosH    = mul(float4(dout.PosW, 1.0f), gViewProj);
	
	return dout;
}

float4 PS(DomainOut pin, 
          uniform int gLightCount, 
		  uniform bool gFogEnabled) : SV_Target
{
	//
	// 중심차분법을 이용해서 접벡터들과 법선 벡터를 추청한다.
	// 해당 정점에서 x의 기울기, z의 기울기를 구하고
	// y에 대한 미분 x --> f(x, y, z) => f(1, dy/dx, 0), 
	// y에 대한 미분 z --> f(x, y, -z) => f(0, dy/dz, -1)을 해서
	// 세계 공간 기준으로 서술된 TBN을 획득한다.
	// z는 텍스쳐 좌표 기준 아래방향이 정방향이라 -z가 맞다.
	//
	
	// 텍스쳐 x좌표 방향으로 한 격자 전, 후의 좌표 획득
	float2 leftTex   = pin.Tex + float2(-gTexelCellSpaceU, 0.0f);
	float2 rightTex  = pin.Tex + float2(gTexelCellSpaceU, 0.0f);
	
	// 텍스쳐 z좌표 방향으로 한 격자 전, 후의 좌표 획득
	float2 bottomTex = pin.Tex + float2(0.0f, gTexelCellSpaceV);
	float2 topTex    = pin.Tex + float2(0.0f, -gTexelCellSpaceV);
	
	// 실제 높이 차이 획득
	float leftY   = gHeightMap.SampleLevel( samHeightmap, leftTex, 0 ).r;
	float rightY  = gHeightMap.SampleLevel( samHeightmap, rightTex, 0 ).r;
	float bottomY = gHeightMap.SampleLevel( samHeightmap, bottomTex, 0 ).r;
	float topY    = gHeightMap.SampleLevel( samHeightmap, topTex, 0 ).r;
	
	// 기울기 구해서 월드 좌표에서 TBN 획득
	float3 tangent = normalize(float3(2.0f*gWorldCellSpace, rightY - leftY, 0.0f));
	// 텍스쳐 좌표와 세계 좌표의 z축 뱡향은 반대
	// 예를 들어 높이맵 텍스쳐 좌표에서 (0,0.1)이 실제 좌표에서 (0, y값, 500)에 대응되면
	// (0,0.2) 이 녀석은 실제 좌표에서 (0, y값, 250)에 대응될 것이다.
	// 왜냐면 텍스쳐 좌표에서 밑으로 내려갈 수록 텍스쳐 좌표 v는 커지지만 세계좌표에서는 z 값이 작아진다.
	// 여기서 topTex은 (0,0.1) 얘가 될 것이고 bottomTex은 (0,0.2) 얘가 될 것이다.
	// 그러면 최종적으로 텍스쳐 축 v 방향의 벡터는 (0,0.2) - (0,0.1) 이렇게 구하게 될 것이고
	// 이를 세계좌표로 옮겨서 진행하면 (0, y값, 250) - (0, y값, 500) 이러면
	// y값은 결과적으로 bottomY - topY 이게 되었고 z값은 음수가 되니 밑과 같다.
	float3 bitan   = normalize(float3(0.0f, bottomY - topY, -2.0f*gWorldCellSpace));
	float3 normalW = cross(tangent, bitan);


	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
	//
	// Texturing
	//
	
	// 텍스쳐 배열에 저장된 텍스쳐 계층들에서 표본들을 추출한다.
	// 이 녀석들의 알파값은 의미가 없다. 
	// 재사용성을 늘리기 위해 혼합 맵 텍스쳐를 따로 사용한다.
	float4 c0 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 0.0f) );
	float4 c1 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 1.0f) );
	float4 c2 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 2.0f) );
	float4 c3 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 3.0f) );
	float4 c4 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 4.0f) ); 
	
	// 혼합 맵에서 알파 성분을 추출한다.
	// 각 r,g,b,a에 각기 다른 회색조 비율이 담겨있다.
	// 이 혼합맵에서 pin.Tex을 사용하는 이유는 높이맵과 혼합 비율이 연동되기 때문이다.
	// 지형이 높아질 수록 눈이 쌓인 지형이라던지 이런 것을 만드는데 효과적이다.
	float4 t  = gBlendMap.Sample( samLinear, pin.Tex ); 
    
    // 계층들을 층층이 혼합한다.
	// 각기 다른 지형을 각기 다른 비율로 섞어 최종 텍스쳐를 뽑아낸다.
    float4 texColor = c0;
    texColor = lerp(texColor, c1, t.r);
    texColor = lerp(texColor, c2, t.g);
    texColor = lerp(texColor, c3, t.b);
    texColor = lerp(texColor, c4, t.a);
 
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
			ComputeDirectionalLight(gMaterial, gDirLights[i], normalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

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

    return litColor;
}

technique11 Light1
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, false) ) );
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, false) ) );
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false) ) );
    }
}

technique11 Light1Fog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, true) ) );
    }
}

technique11 Light2Fog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true) ) );
    }
}

technique11 Light3Fog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true) ) );
    }
}
