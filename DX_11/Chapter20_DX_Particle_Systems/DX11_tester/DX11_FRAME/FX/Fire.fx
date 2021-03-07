//=============================================================================
// Fire.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Fire particle system.  Particles are emitted directly in world space.
//=============================================================================


//***********************************************
// GLOBALS                                      *
//***********************************************

cbuffer cbPerFrame
{
	float3 gEyePosW;
	
	// 방출 위치나 방향이 다른 경우를 위한 변수들.
	float3 gEmitPosW;
	float3 gEmitDirW;
	
	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj; 
};

cbuffer cbFixed
{
	// 입자의 가속을 위한 알짜 상수 가속도.
	// y축으로 가속
	float3 gAccelW = {0.0f, 7.8f, 0.0f};
	
	// 텍스쳐를 사각형 전체에 입히는 텍스쳐 좌표들.
	// 점 입자를 사각형으로 확장할 때 쓰임. (빌보드 했을 때랑 같은 원리)
	float2 gQuadTexC[4] = 
	{
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};
};
 
// 입자 텍스쳐 적용을 위한 텍스쳐 배열
Texture2DArray gTexArray;

// 셰이더에서 난수를 얻는 데 쓰이는 무작위 텍스쳐.
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

BlendState AdditiveBlending // 가산 혼합
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

//***********************************************
// 보조 함수들                           
//***********************************************
float3 RandUnitVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);
	
	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	// project onto unit sphere
	return normalize(v);
}
 
//***********************************************
// 스트림 출력 전용 기법 (렌더링 X)
//***********************************************

#define PT_EMITTER 0 // 방출기면 0
#define PT_FLARE 1 // 입자면 1
 
struct Particle
{
	float3 InitialPosW : POSITION; // 입자 시작 위치
	float3 InitialVelW : VELOCITY; // 입자 시작 속도
	float2 SizeW       : SIZE; // 입자 크기
	float Age          : AGE; // 입자의 나이 (입자가 시간에 따라 이동하기에 필요)
	uint Type          : TYPE; // 입자의 방출기인지 방출이 된 입자인지 종류
};

// 스트림 출력 전용이기에 정점 쉐이더는 할 일이 없다.
// 일단 주어진 정점이 없고 기하 쉐이더에서 만들어 내야 하기 때문이다.
Particle StreamOutVS(Particle vin)
{
	return vin;
}

// 스트림 출력 전용 기하 셰이더는 새 입자의 방출과 기존 입자의
// 파괴만 담당한다. 입자 시스템마다 입자의 생성, 파괴 규칙이 다를 것이므로
// 이 부분의 구현 역시 입자 시스템마다 다를 필요가 있다.
[maxvertexcount(2)]
void StreamOutGS(point Particle gin[1], 
                 inout PointStream<Particle> ptStream)
{	
	gin[0].Age += gTimeStep; // 입자의 나이 추가 (gTimeStep == deltaTime)
	
	if ( gin[0].Type == PT_EMITTER )
	{	
		// 새 입자를 생성할 시간이 되었는가?
		if( gin[0].Age > 0.005f )
		{
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;
			
			Particle p;
			p.InitialPosW = gEmitPosW.xyz; // 방출되는 입자는 방출기의 위치
            p.InitialVelW = 4.0f * vRandom; // 입자의 속도는 랜덤한 값에 4배
			p.SizeW       = float2(3.0f, 3.0f); // 입자의 크기 조절
			p.Age         = 0.0f; // 방출 입자는 초기 나이가 0
			p.Type        = PT_FLARE; // 방출기가 아닌 입자라고 명시
			
			ptStream.Append(p); // 입자를 점으로 방출
			
			// 방출하나 했으면 다시 초기화하고 0.005까지 대기
			gin[0].Age = 0.0f;
		}
		
		// 방출된 입자는 유지
		ptStream.Append(gin[0]);
	}
	else
	{
		// 여기서 입자를 유지할 조건들을 지정한다.
		// 구체적인 조건은 입자 시스템마다 다를 수 있다.
		// 불입자는 나이가 1이 넘어가면 사라진다.
		if( gin[0].Age <= 1.0f )
			ptStream.Append(gin[0]);
	}
}

GeometryShader gsStreamOut = ConstructGSWithSO( 
	CompileShader( gs_5_0, StreamOutGS() ), 
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x" );
	
technique11 StreamOutTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, StreamOutVS() ) );
        SetGeometryShader( gsStreamOut );
        
        // 픽셀 쉐이더 비활성화
        SetPixelShader(NULL);
        
        // 깊이 버퍼 비활성화
		// 정점만 생성해 놓는 것이라 깊이 버퍼를 쓰고 지우고 판단할 이유가 없다.
		// 처음 정점 생성하고 CreateBuffer 하는 경우 정점에 대한 좌표만 정하고 넣는 것을 생각해보면 된다.
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//***********************************************
// 렌더링 기법
//***********************************************

struct VertexOut
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
	uint   Type  : TYPE;
};

VertexOut DrawVS(Particle vin)
{
	VertexOut vout;
	
	float t = vin.Age;
	
	// 시간에 따른 거리(위치) 등가속도 운동식
    vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitialVelW + vin.InitialPosW;
	
	// 시간에 따른 색상 감소.
	// smoothstep에 대한 함수는 http://egloos.zum.com/chulin28ho/v/5324893
	// 링크에 그림과 잘 설명이 되어있다. 
	// 대략 스무스하게 특정 구간에서 값을 뜯어주는 함수라고 보면 된다.
	// opacity 알파 값이 시간에 따라서 옅어지므로 불 입자는 
	// 위로 상승하면서 점점 희미해지는 효과가 생긴다.
	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t/1.0f);
	vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);
	
	vout.SizeW = vin.SizeW;
	vout.Type  = vin.Type;
	
	return vout;
}

struct GeoOut
{
	float4 PosH  : SV_Position;
	float4 Color : COLOR;
	float2 Tex   : TEXCOORD;
};

// 렌더링용 기하 셰이더는 그냥 점을 카메라를 향한 사각형으로 확장한다.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1], 
            inout TriangleStream<GeoOut> triStream)
{	
	// 방출기 입자는 그리지 않는다.
	if( gin[0].Type != PT_EMITTER )
	{
		//
		// 빌보드가 카메라를 향하게 하는 세계 행렬을 계산한다.
		//
		float3 look  = normalize(gEyePosW.xyz - gin[0].PosW); // 카메라가 보는 방향의 반대
		float3 right = normalize(cross(float3(0,1,0), look)); // 우측 벡터 축
		float3 up    = cross(look, right); // 외적으로 위 축까지 구한다.
		
		//
		// 사각형을 구성하는 삼각형 띠 정점들을
		// 계산한다. (빌보드와 같은 원리)
		//
		float halfWidth  = 0.5f*gin[0].SizeW.x;
		float halfHeight = 0.5f*gin[0].SizeW.y;
	
		float4 v[4];
		v[0] = float4(gin[0].PosW + halfWidth*right - halfHeight*up, 1.0f);
		v[1] = float4(gin[0].PosW + halfWidth*right + halfHeight*up, 1.0f);
		v[2] = float4(gin[0].PosW - halfWidth*right - halfHeight*up, 1.0f);
		v[3] = float4(gin[0].PosW - halfWidth*right + halfHeight*up, 1.0f);
		
		//
		// 이 정점들을 세계 공간으로 변환하고, 하나의 삼각형 띠로서
		// 출력한다.
		//
		GeoOut gout;
		[unroll]
		for(int i = 0; i < 4; ++i)
		{
			gout.PosH  = mul(v[i], gViewProj);
			gout.Tex   = gQuadTexC[i];
			gout.Color = gin[0].Color;
			triStream.Append(gout);
		}
	}
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
    return gTexArray.Sample(samLinear, float3(pin.Tex, 0)) * pin.Color; // 텍스쳐 배열에서 첫번째 텍스쳐
}

technique11 DrawTech
{
    pass P0
    {
        SetVertexShader(   CompileShader( vs_5_0, DrawVS() ) );
        SetGeometryShader( CompileShader( gs_5_0, DrawGS() ) );
        SetPixelShader(    CompileShader( ps_5_0, DrawPS() ) );
        
        SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff); // 가산 혼합 적용
        SetDepthStencilState( NoDepthWrites, 0 );
    }
}