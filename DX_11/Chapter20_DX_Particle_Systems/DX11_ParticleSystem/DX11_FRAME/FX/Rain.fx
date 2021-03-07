//=============================================================================
// Rain.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Rain particle system.  Particles are emitted directly in world space.
//=============================================================================


//***********************************************
// 전역 설정
//***********************************************

cbuffer cbPerFrame
{
	float3 gEyePosW;
	
	// 방출 위치나 방향이 다른 경우를 위한 변수들.
	float3 gEmitPosW; // 방출 위치는 카메라 위치
	float3 gEmitDirW;
	
	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj; 
};

cbuffer cbFixed
{
	// 입자의 가속을 위한 알짜 상수 가속도.
	// 비가 사선으로 내린다.
	float3 gAccelW = {-1.0f, -9.8f, 0.0f};
};
 
// 입자 텍스처 적용을 위한 텍스처 배열
Texture2DArray gTexArray;

// 쉐이더에서 난수를 얻는데 쓰이는 난수 텍스쳐
Texture1D gRandomTex;
 
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};


//***********************************************
// 보조 함수
//***********************************************
float3 RandUnitVec3(float offset)
{
	// 게임 시간 더하기 오프셋을 무작위 텍스처 추출을 위한 텍스처 좌표로 사용한다.
	float u = (gGameTime + offset);
	
	// 성분들의 범위는 [-1,1]
    float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz; // gRandomTex 얘는 밉맵이 단 하나라 레벨 0을 지정
	
	// 단위 구로 투영
	return normalize(v);
}

float3 RandVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);
	
	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	return v;
}
 
//***********************************************
// 스트림 출력
//***********************************************

#define PT_EMITTER 0
#define PT_FLARE 1
 
struct Particle
{
	float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW       : SIZE;
	float Age          : AGE;
	uint Type          : TYPE;
};
  
Particle StreamOutVS(Particle vin)
{
	return vin;
}

// 스트림 출력 전용 기하 셰이더는 새 입자의 방출과 기존 입자의
// 파괴만 담당한다. 입자 시스템마다 입자의 생성, 파괴 규칙이 다를 것이므로
// 이 부분의 구현 역시 입자 시스템마다 다를 필요가 있다.
[maxvertexcount(6)]
void StreamOutGS(point Particle gin[1], 
                 inout PointStream<Particle> ptStream)
{	
	gin[0].Age += gTimeStep;

	if( gin[0].Type == PT_EMITTER )
	{	
		// 새 입자를 방출할 시간이 되었는가?
		if( gin[0].Age > 0.002f )
		{
			for(int i = 0; i < 5; ++i) // 빗방울 5개 생성
			{
				// 카메라 위쪽에 빗방울들을 분포시킨다.
                float3 vRandom = 35.0f * RandVec3((float) i / 5.0f);
				vRandom.y = 20.0f; // 카메라 위치보다 위로 항상 20 위
			
				Particle p;
                p.InitialPosW = gEmitPosW.xyz + vRandom; // gEmitPosW는 사실상 카메라 위치로 Main.cpp에서 설정된다.
				p.InitialVelW = float3(0.0f, 0.0f, 0.0f); // 빗방울의 처음 속도는 0
				p.SizeW       = float2(1.0f, 1.0f); // 빗방울 크기는 1
				p.Age         = 0.0f;
				p.Type        = PT_FLARE;
			
				ptStream.Append(p);
			}
			
			// 방출 시간 초기화
			gin[0].Age = 0.0f;
		}

		// 방출 정점 다시 설정
		ptStream.Append(gin[0]);
	}
	else
	{
		// 시간이 3초 동안 빗방울 유지
		if( gin[0].Age <= 3.0f )
			ptStream.Append(gin[0]);
	}
}

// 스트림 출력
GeometryShader gsStreamOut = ConstructGSWithSO( 
	CompileShader( gs_5_0, StreamOutGS() ), 
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x" );
	
technique11 StreamOutTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, StreamOutVS() ) );
        SetGeometryShader( gsStreamOut );
        
        // 스트림 출력 전용을 위해 픽셀 셰이더를 비활성화한다.
        SetPixelShader(NULL);
        
        // 스트림 출력 전용을 위해서는 깊이 버퍼도 비활성화해야 한다.
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//***********************************************
// 렌더링 기법
//***********************************************

struct VertexOut
{
	float3 PosW  : POSITION;
	uint   Type  : TYPE;
};

VertexOut DrawVS(Particle vin)
{
	VertexOut vout;
	
	float t = vin.Age;
	
	// 시간에 따른 등가속도 운동 거리 공식
    vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitialVelW + vin.InitialPosW;
	
	vout.Type  = vin.Type;
	
	return vout;
}

struct GeoOut
{
	float4 PosH  : SV_Position;
	float2 Tex   : TEXCOORD;
};

// 빗방울은 그냥 점을 선으로 확장
[maxvertexcount(2)]
void DrawGS(point VertexOut gin[1], 
            inout LineStream<GeoOut> lineStream)
{
	// 방출기는 그리지 않고 방출기 아닌 것을 그린다.
    if (gin[0].Type != PT_EMITTER)
    {
		// 선분이 가속도 방향으로 기울어지게 한다.
        float3 p0 = gin[0].PosW;
        float3 p1 = gin[0].PosW + 0.07f * gAccelW;
		
        GeoOut v0;
        v0.PosH = mul(float4(p0, 1.0f), gViewProj);
        v0.Tex = float2(0.0f, 0.0f);
        lineStream.Append(v0);
		
        GeoOut v1;
        v1.PosH = mul(float4(p1, 1.0f), gViewProj);
        v1.Tex = float2(1.0f, 1.0f);
        lineStream.Append(v1);
    }
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
	return gTexArray.Sample(samLinear, float3(pin.Tex, 0));
}

technique11 DrawTech
{
    pass P0
    {
        SetVertexShader(   CompileShader( vs_5_0, DrawVS() ) );
        SetGeometryShader( CompileShader( gs_5_0, DrawGS() ) );
        SetPixelShader(    CompileShader( ps_5_0, DrawPS() ) );
        
        SetDepthStencilState( NoDepthWrites, 0 );
    }
}