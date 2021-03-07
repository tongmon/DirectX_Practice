// 방출기가 다른 방출기를 방출하는 효과 파일을 연습했다.
// 실행해보면 입자들이 여러 색으로 위로 분열하면서 올라갈 것이다.
// 여기서 애를 먹은 부분이 난수를 다루는 경우였는데
// 처음에 방출기에서 입자를 생성하면 여러개의 다양한
// 시작 속도를 가진 입자들이 생성될 것이다.
// 근데 이 입자들이 다시 방출기 역할을 하게 될 때
// 이 초반에 바로 생성된 초창기 두번째 방출기들의 난수 생성 방식을 모두 같게 하면
// 동일한 값의 난수들이 발생해서 동일한 속도로 계속 분열되는 현상을 볼 수 있다.
// 이렇게 동일하면 사실적이지 않고 의도한 바가 아니다.
// 이를 해결하기 위해 각 입자들의 서로 다른 차이점을 생각해봤더니 시작 속도가 서로 다른 상태였다.
// 따라서 시작 속도를 오프셋으로 추가하여 각 입자들이 서로 다른 난수를 생성할 수 있게 했다.
// 그리고 또 실수한 부분이 있는데 [maxvertexcount(64)] 기하쉐이더에서 이 부분의 정점 개수를 잘 확인하자...
// 입자 생성시에 루프가 5번 돌았는데 2개밖에 생성이 안되어서 계속 찾다가 보니 저 값이 2로 되어있더라...

//***********************************************
// 전역 변수들
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
    float3 gAccelW = { 0.0f, 7.8f, 0.0f };
    float N = 10;
    float M = 3;
	
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

float3 RandVec3(float offset)
{
	// Use game time plus offset to sample random texture.
    float u = (gGameTime + offset);
	
	// coordinates in [-1,1]
    float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
    return v;
}
 
//***********************************************
// 스트림 출력 전용 기법 (렌더링 X)
//***********************************************

#define PT_EMITTER 0 // 시작 방출기 0
#define PT_EMITTER2 1 // 새로운 방출기 1
 
struct Particle
{
    float3 InitialPosW : POSITION; // 입자 시작 위치
    float3 InitialVelW : VELOCITY; // 입자 시작 속도
    float2 SizeW : SIZE; // 입자 크기
    float Age : AGE; // 입자의 나이 (입자가 시간에 따라 이동하기에 필요)
    uint Type : TYPE; // 입자의 방출기인지 방출이 된 입자인지 종류
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
[maxvertexcount(64)]
void StreamOutGS(point Particle gin[1],
                 inout PointStream<Particle> ptStream)
{
    gin[0].Age += gTimeStep; // 입자의 나이 추가 (gTimeStep == deltaTime)
	
    if (gin[0].Type == PT_EMITTER)
    {
		// 새 입자를 생성할 시간이 되었는가?
        if (gin[0].Age > 0.05f)
        {
            for (int i = 0; i < N; i++)
            {
                float3 vRandom = RandUnitVec3((float) i / N); // 초반에 처음 생성하는 난수는 그냥 이렇게 해도 된다.
                vRandom.x *= 2.0f;
                vRandom.z *= 2.0f;
                vRandom.y = abs(vRandom.y);
			
                Particle p;
                p.InitialPosW = gEmitPosW.xyz; // 방출되는 입자는 방출기의 위치
                p.InitialVelW = vRandom; // 입자의 속도는 랜덤한 값에 4배
                p.SizeW = float2(2.0f, 2.0f); // 입자의 크기 조절
                p.Age = 0.0f; // 방출 입자는 초기 나이가 0
                p.Type = PT_EMITTER2; // 방출기가 아닌 입자라고 명시
			
                ptStream.Append(p); // 입자를 점으로 방출
            }
        }
        else
        {
            ptStream.Append(gin[0]); // 입자 방출했으면 제거
        }
    }
    else // PT_EMITTER2
    {
        // 난수 생성시에 시간의 영향을 받지 않고 오직 초반 속도에만 영향을 받아 난수 생성
        // 이 난수는 입자를 생성하는 시기를 결정
        float randTime = abs(RandVec3(gin[0].InitialVelW.y - gGameTime).x);
        if (gin[0].Age > min(10 * randTime, 2))
        {
            for (int i = 0; i < M; i++)
            {
                // 여기서 그냥 RandUnitVec3((float) i / M); 이렇게 해버리면
                // 같은 시간에 생성된 입자들은 모두 같은 난수 속도를 가지는 하위 입자들을 생성한다.
                // 따라서 해당 입자의 속도(입자마다 모두 다름)를 변수로 하여 추가적인 변인을 준다.
                float3 vRandom = RandUnitVec3((float) i / M + gin[0].InitialVelW.y);
                vRandom.x *= 0.5f;
                vRandom.z *= 0.5f;
                vRandom.y = -abs(vRandom.y);
			
                float t = gin[0].Age;
                Particle p;
                // 새로 방출되는 입자의 시작 위치는 새로 계산한다.
                p.InitialPosW = 0.5f * t * t * gAccelW + t * gin[0].InitialVelW + gin[0].InitialPosW; // 방출기에서 방출된 또다른 방출기 입자 위치
                p.InitialVelW = 4.0f * vRandom; // 입자의 속도는 랜덤한 값에 4배
                p.SizeW = float2(2.0f, 2.0f); // 입자의 크기 조절
                p.Age = 0.0f; // 방출 입자는 초기 나이가 0
                p.Type = PT_EMITTER2; // 방출기는 또 다른 방출기 방출
			
                ptStream.Append(p); // 입자를 점으로 방출
            }
        }
        else
        {
            ptStream.Append(gin[0]); // 입자 방출했으면 제거
        }
    }
}

GeometryShader gsStreamOut = ConstructGSWithSO(
	CompileShader(gs_5_0, StreamOutGS()),
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x");
	
technique11 StreamOutTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, StreamOutVS()));
        SetGeometryShader(gsStreamOut);
        
        // 픽셀 쉐이더 비활성화
        SetPixelShader(NULL);
        
        // 깊이 버퍼 비활성화
		// 정점만 생성해 놓는 것이라 깊이 버퍼를 쓰고 지우고 판단할 이유가 없다.
		// 처음 정점 생성하고 CreateBuffer 하는 경우 정점에 대한 좌표만 정하고 넣는 것을 생각해보면 된다.
        SetDepthStencilState(DisableDepth, 0);
    }
}

//***********************************************
// 렌더링 기법
//***********************************************

struct VertexOut
{
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
    float4 Color : COLOR;
    uint Type : TYPE;
};

VertexOut DrawVS(Particle vin)
{
    VertexOut vout;
	
    float t = vin.Age;
	
	// 시간에 따른 거리(위치) 등가속도 운동식
    vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitialVelW + vin.InitialPosW;
	
    // 색상도 랜덤으로 설정, 게임 시간에 따른 난수가 아닌 해당 정점의 속도에 따른 난수를 획득
    float3 randCol = RandUnitVec3(vin.InitialVelW.y - gGameTime);
    randCol = abs(randCol);
    vout.Color = float4(randCol, 1.0);
	
    vout.SizeW = vin.SizeW;
    vout.Type = vin.Type;
	
    return vout;
}

struct GeoOut
{
    float4 PosH : SV_Position;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

// 렌더링용 기하 셰이더는 그냥 점을 카메라를 향한 사각형으로 확장한다.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1],
            inout TriangleStream<GeoOut> triStream)
{
	// 방출기 입자는 그리지 않는다.
	//
	// 빌보드가 카메라를 향하게 하는 세계 행렬을 계산한다.
	//
    float3 look = normalize(gEyePosW.xyz - gin[0].PosW); // 카메라가 보는 방향의 반대
    float3 right = normalize(cross(float3(0, 1, 0), look)); // 우측 벡터 축
    float3 up = cross(look, right); // 외적으로 위 축까지 구한다.
		
	//
	// 사각형을 구성하는 삼각형 띠 정점들을
	// 계산한다. (빌보드와 같은 원리)
	//
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
	
    float4 v[4];
    v[0] = float4(gin[0].PosW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].PosW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].PosW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].PosW - halfWidth * right + halfHeight * up, 1.0f);
		
	//
	// 이 정점들을 세계 공간으로 변환하고, 하나의 삼각형 띠로서
	// 출력한다.
	//
    GeoOut gout;
	[unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(v[i], gViewProj);
        gout.Tex = gQuadTexC[i];
        gout.Color = gin[0].Color;
        triStream.Append(gout);
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
        SetVertexShader(CompileShader(vs_5_0, DrawVS()));
        SetGeometryShader(CompileShader(gs_5_0, DrawGS()));
        SetPixelShader(CompileShader(ps_5_0, DrawPS()));
        
        SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff); // 가산 혼합 적용
        SetDepthStencilState(NoDepthWrites, 0);
    }
}