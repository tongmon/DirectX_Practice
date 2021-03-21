//=============================================================================
// Ssao.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// SSAO 맵을 계산하는 fx.
//=============================================================================

cbuffer cbPerFrame
{
	float4x4 gViewToTexSpace; // Proj * Texture, 투영 행렬 곱하기 텍스쳐 행렬
	float4   gOffsetVectors[14];
	float4   gFrustumCorners[4];

	// 차폐 판정에 쓰이는 상수들.
	float    gOcclusionRadius    = 0.5f;
	float    gOcclusionFadeStart = 0.2f;
	float    gOcclusionFadeEnd   = 2.0f;
	float    gSurfaceEpsilon     = 0.05f;
};
 
// 비수치 값들은 상수 버퍼에 추가할 수 없다.
Texture2D gNormalDepthMap; // xyz는 시점에서 본 픽셀의 법선, 알파값(w)은 깊이
Texture2D gRandomVecMap; // 난수 저장 텍스쳐 배열
 
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// 법선 깊이 맵 바깥의 표본을 추출하는 경우 잘못된
	// 차폐도가 나오지 않도록 아주 먼 깊이 값을 설정한다.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f); // 맨 마지막 알파값이 차폐도
};

SamplerState samRandomVec
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU  = WRAP;
    AddressV  = WRAP;
};

struct VertexIn
{
	float3 PosL            : POSITION;
	float3 ToFarPlaneIndex : NORMAL;
	float2 Tex             : TEXCOORD;
};

struct VertexOut
{
    float4 PosH       : SV_POSITION;
    float3 ToFarPlane : TEXCOORD0;
	float2 Tex        : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// 이미 NDC 공간에 있다. 
	// 왜냐면 FullScreen 버퍼 생성하고 화면에 막을 씌운 상태의 정점 4개만 들어오기 때문
	vout.PosH = float4(vin.PosL, 1.0f);

	// 법선의 x 성분에는 절두체 먼 평면 꼭짓점의 색인이 저장되어 있다.
	// ToFarPlaneIndex 이름 땜시 혼동이 올 수 있는데 기존 Normal과 같은 역할의 변수다.
	vout.ToFarPlane = gFrustumCorners[vin.ToFarPlaneIndex.x].xyz;

	// 텍스쳐 좌표는 그대로 픽셀 셰이더에 넘겨준다.
	vout.Tex = vin.Tex;
	
    return vout;
}

// 표본점 q가 점 p를 얼마나 가리는지를 distZ (깊이 차이)의 함수로서 계산한다.
// distZ = | depth(p) - depth(q) |
float OcclusionFunction(float distZ)
{
	//
	// 만일 depth(q) 가  depth(p) 뒤에 있다면 q는 p를 가릴 수 없다.
	// 애초에 AO 기법 자체가 해당 정점의 법선 방향으로의 반원을 그린다음에
	// 무작위 레이져를 쏴서 차폐도를 구하는 것인데 반원을 넘어간 q(그니까 뒤에 있는 q)는 p를 가릴 수 없다.
	// 또 depth(q) 가 depth(p) 와 너무 가까워도 q가 p를 가리지 않는 것으로 한다.
	// 거의 직각에 가까운 평면에서 p, q와의 거리가 엄청 가까울 수가 있는데 이를 차폐도에 반영하는 건 좀 이상해질 수 있다.
	// 따라서 Epsilon 값보다는 p, q 거리 차이가 커야한다. (즉 그만큼 q가 p 앞에 위치해야 차폐도에 반영이 된다.)
	//
	// 차폐도 계산에는 다음과 같은 함수를 사용한다.
	// 
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//
	
	float occlusion = 0.0f;
	if(distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		
		// distZ가 gOcclusionFadeStart에서 gOcclusionFadeEnd로 증가함에
		// 따라 차폐도를 1에서 0으로 선형 감소시킨다.
		// gOcclusionFadeEnd를 넘어가서까지 거리차이가 나면 AO에 따른 계수를 매기기가
		// 부적절하다고 판단하는 것.
        occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
    }
	
	return occlusion;	
}

float4 PS(VertexOut pin, uniform int gSampleCount) : SV_Target
{
	// p -- 지금 주변광 차폐를 계산하고자 하는 픽셀에 해당하는 점
	// n -- p에서의 법선 벡터
	// q -- p 주변의 한 무작위 점
	// q의 의미가 중요한데 q는 그냥 점이다. 
	// p에서 뻗어나간 무작위 레이져가 다른 물체와 부딫혀서 생긴 접점이 아니라 그냥 점이다.
	// SSAO에서는 이 점을 통해 AO 수치를 추정해야 한다.
	// 기존의 AO 방식을 실제로 구현해서 써먹으려면 로딩 시간이 미친듯이 길어져서
	// SSAO를 사용하는데 SSAO는 AO 값 획득은 빠르지만 추정값이라는 것이 단점이다.
	// r -- p를 가릴 가능성이 있는 잠재적 차폐점.

	// 이 픽셀의 시야 공간 법선과 z 성분을 가져온다. 지금 렌더링되는
	// 화면 전체 사각형의 텍스쳐 좌표들은 이미 uv 공간에 있다.
	// normalDepth 여기에 들어있는 녀석들은 World * View 밖에 안 곱해져 있다.
	// 따라서 z 범위가 0 ~ 1 이 아니라 가지 각색이다.
	// 코드에서는 0.1 ~ 10000 이렇게 짰을 것이다.
	float4 normalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
 
	float3 n = normalDepth.xyz; // 해당 픽셀의 법선 획득
	float pz = normalDepth.w; // 깊이 획득

	//
	// 완전한 시야 공간 위치 (x, y, z) 를 재구축한다.
	// 우선 p = t * pin.ToFarPlane 를 만족하는 t를 구한다.
	// p.z = t * pin.ToFarPlane.z
	// t = p.z / pin.ToFarPlane.z
	// ToFarPlane 이 녀석의 범위도 normalDepth 얘와 같다.
	//
    float3 p = (pz / pin.ToFarPlane.z) * pin.ToFarPlane;
	
	// 무작위 벡터를 추출해서 [0,1] --> [-1,1]로 사상한다.
    float3 randVec = 2.0f * gRandomVecMap.SampleLevel(samRandomVec, 4.0f * pin.Tex, 0.0f).rgb - 1.0f;

	float occlusionSum = 0.0f;
	
	// p 주변의 이웃 표본점들을 n 방향의 반구에서 추출한다. AO 알고리즘
	[unroll]
	for(int i = 0; i < gSampleCount; ++i)
	{
		// 미리 만들어 둔 상수 오프셋 벡터들은 고르게 분포되어 있다.
		// 오프셋 벡터들은 같은 방향으로 뭉쳐 있지 않다
		// 이들을 하나의 무작위 벡터를 기준으로 반사시키면 고르게 분포된 무작위 벡터들이
		// 만들어진다.
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);
	
		// 오프셋 벡터가 (p, n) 으로 정의된 평면의 뒤쪽을 향하고 있으면
		// 방향을 반대로 뒤집는다.
		// 그니까 반구쪽이 아니라 반대쪽 반구 방향이면 이 방향을 올바르게 교정함.
		// 둔각이면 내적 값이 음수임을 이용
		float flip = sign( dot(offset, n) );
		
		// p 주변에서 차폐 반지름 이내의 무작위 점 q를 선택한다. (반직선)
		float3 q = p + flip * gOcclusionRadius * offset;
		
		// q를 투영해서 투명 텍스쳐 좌표를 구한다.
		// q는 이미 시야 좌표계에서 정의되어 있으니 투영 행렬만 곱해준다. (물론 텍스쳐 범위 변환 행렬도 곱한다.)
		float4 projQ = mul(float4(q, 1.0f), gViewToTexSpace);
		projQ /= projQ.w; // 동차 나누기

		// 시점에서 q로의 반직선에서 시점에 가장 가까운 픽셀의 깊이를 구한다.
		// 이것이 q의 깊이는 아니고 q는 그냥 p 근처의 임의의 점이며,
		// 장면의 물체가 아닌 빈 공간에 있는 점일 수도 있다.
		// 가장 가까운 깊이는 깊이 맵에서 추출한다.

		// projQ 얘의 xy 좌표는 텍스쳐 변환까지 완료된 상태라
		// samNormalDepth 얘와 1:1로 매칭된다.
		float rz = gNormalDepthMap.SampleLevel(samNormalDepth, projQ.xy, 0.0f).a;

		// 완전한 시야 공간 위치 r = (rx, ry, rz)를 재구성한다.
		// r은 q를 지나는 반직선에 있으므로, r = t * q를 만족하는 t가 존재한다.
		// r.z = t*q.z ==> t = r.z / q.z
		// 간단한 투영 비례 식이다. (삼각형 변 비례)

		float3 r = (rz / q.z) * q;
		
		//
		// r이 p를 가리는지 판정.
		//   * 내적 dot(n, normalize(r - p)) 는 잠재적 차폐점 r이 평면
		//     plane(p,n) 앞 쪽으로 얼마나 앞에 있는지를 나타낸다.
		//     더 앞에 있을수록(평면에 가까울 수록) 차폐도 가중치를 더 크게 잡는다.
		//     이렇게 하면 r이 시선과 직각인 평면 (p, n)에 있을 때 시점 기준의 깊이 값 차이 때문에
		//	   r이 p를 가린다고 잘못 판정하는 상황도 방지된다.
		//   * 차폐도는 현재 점 p와 차폐점 r 사이의 거리에 의존한다.
		//     r 이 p에서 너무 멀리 있으면 p를 가리지 않는 것으로 간주된다.
		// 
		
		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f); // 가중치이고 법선과 p->r 벡터의 각이 좁으면 좁을 수록 커진다.
		float occlusion = dp * OcclusionFunction(distZ); // 거리조건까지 따진 최종 차폐도를 획득
		
		occlusionSum += occlusion; // 각 랜덤한 광선마다 도출된 차폐도 합산
	}
	
	occlusionSum /= gSampleCount; // 샘플 수만큼 나누어서 평균 도출
	
	float access = 1.0f - occlusionSum; // 차폐도의 반대는 빛을 받는 만큼을 뜻한다.

	// SSAO가 극명한 효과를 내도록, 적절히 거듭제곱을 해준다.
	// SSAO 맵의 대비가 제곱이 계속될 수록 커진다.
	// 이렇게 해서 시점 기준으로 모든 픽셀에 대한 차폐율 계산이 완료되었다.
	return saturate(pow(access, 4.0f));
}

technique11 Ssao
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(14) ) );
    }
}
 