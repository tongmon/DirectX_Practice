//=============================================================================
// VecAdd.fx by Frank Luna (C) 2011 All Rights Reserved.
//=============================================================================

// 두 구조적 버퍼 gInputA, gInputB 에 담긴 값들을 합한다.

struct Data
{
	float3 v1;
	float2 v2;
};

StructuredBuffer<Data> gInputA;
StructuredBuffer<Data> gInputB;
RWStructuredBuffer<Data> gOutput;

// 스레드는 가로로 32개 배치, 즉 일차원적이다. 
// 이유는 메인 소스의 mNumElements의 수가 32개여서...
[numthreads(32, 1, 1)]
void CS(int3 dtid : SV_DispatchThreadID)
{
	gOutput[dtid.x].v1 = gInputA[dtid.x].v1 + gInputB[dtid.x].v1;
	gOutput[dtid.x].v2 = gInputA[dtid.x].v2 + gInputB[dtid.x].v2;
}

technique11 VecAdd
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, CS() ) );
    }
}
