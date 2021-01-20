//=============================================================================
// Blur.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs a separable blur with a blur radius of 5.  
//=============================================================================

cbuffer cbSettings
{
	float gWeights[11] = // 흐리기 가중치들
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
	};
};

cbuffer cbFixed
{
	static const int gBlurRadius = 5; // 흐리기 픽셀 반지름 5개 텍셀 + 현재 텍셀 + 5개 텍셀
};

Texture2D gInput; // 읽기 전용 자원
RWTexture2D<float4> gOutput; // 읽기 쓰기 모두 가능한 출력 자원, UAV 연결

#define N 256
#define CacheSize (N + 2*gBlurRadius)
// 같은 그룹 내에서 공유하는 공유 메모리 선언, 색상값 4개라 float4
// gInput에서 그때 그때 텍셀 값을 추출해 사용하지 않고 여기에 저장해 사용하는 이유는
// 공유 메모리 접근이 훨씬 빠르기에 더 효율적이라 그렇다.
// 크기는 흐리기 처리에 알맞은 R + N + R 양쪽에 반지름 만큼 추가
groupshared float4 gCache[CacheSize];

[numthreads(N, 1, 1)] // 그룹 하나당 스레드 256개
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// 대역폭을 줄이기 위해 지역 스레드 저장소를 채운다.
	// 흐리기 반지름 때문에, 픽셀 N개를 흐리려면 N + 2 * 흐리기 반지름
	// 만큼의 픽셀을 읽어야 한다.
	//

	// 이 스레드 그룹은 N개의 스레드를 실행한다.
	// 여분의 픽셀 2 * 흐리기 반지름 개를 위해, 
	// 2 * 흐리기 반지름 만큼의 스레드가 픽셀을 하나 더 추출하게 한다.
	
	// 현재 스레드 ID가 반지름보다 작으면 흐림처리 시에 256 범위를 벗어나는
	// 텍셀이 필요하다. 정확히 말하면 더 왼쪽 부분이 필요해서
	// 밑과 같은 작업이 수행된다.
	if(groupThreadID.x < gBlurRadius)
	{
		// 이미지 왼쪽 경계 바깥의 표본을 왼쪽 경계로 한정
		// 현 위치에서 반지름을 뺀 위치가 텍셀 범위 내면 dispatchThreadID.x - gBlurRadius 부분 아니면 경계 부분
        int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	
	// 클 때도 마찬가지로 더 오른쪽 부분이 필요해지기에
	// 밑의 작업 수행
	if(groupThreadID.x >= N-gBlurRadius)
    {
		// 이미지 오른쪽 경계 바깥의 표본을 오른쪽 경계로 한정
		int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x-1);
		gCache[groupThreadID.x+2*gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	// 이미지 범위 밖의 표본을 이미지 경계로 한정한다.
	// 이미지 크기를 벗어나서까지 스레드가 존재하는 경우가 많다.
	// 이 예제도 가로 크기를 256으로 나누었기에 가로 크기가 256의 배수가 아니라면
	// 나머지가 있을 테고 그 나머지 만큼의 스레드에 이미지 밖의 색상이 기록되면 안되기에
	// min(dispatchThreadID.xy, gInput.Length.xy-1) 이 연산이 필요하다.
	gCache[groupThreadID.x+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	// 다른 그룹 내의 스레드들도 여기까지 진행이 끝나야 다음 단계로 넘어가게
	// 동기화 시킨다.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// 흐림 처리 진행
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	[unroll]
	for(int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
        // gCache[]에 groupThreadID.x 부터 반지름 왼쪽이고
		// groupThreadID.x + 2*R 부터 반지름 오른쪽이라 인덱스를 다시 만든다.
		int k = groupThreadID.x + gBlurRadius + i; // 인덱스 재설정
		
		blurColor += gWeights[i+gBlurRadius]*gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}

// 세로 방향 흐리기도 방향만 다를 뿐 그냥 똑같다.
[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// 대역폭을 줄이기 위해 지역 스레드 저장소를 채운다.
	// 흐리기 반지름 때문에, 픽셀 N개를 흐리려면 N + 2 * 흐리기 반지름
	// 만큼의 픽셀을 읽어야 한다.
	//
	
	// 이 스레드 그룹은 N개의 스레드를 실행한다.
	// 여분의 픽셀 2 * 흐리기 반지름 개를 위해, 
	// 2 * 흐리기 반지름 만큼의 스레드가 픽셀을 하나 더 추출하게 한다.
	
	// 현재 스레드 ID가 반지름보다 작으면 흐림처리 시에 256 범위를 벗어나는
	// 텍셀이 필요하다. 정확히 말하면 더 왼쪽 부분이 필요해서
	// 밑과 같은 작업이 수행된다.
	if(groupThreadID.y < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
	}
	if(groupThreadID.y >= N-gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = min(dispatchThreadID.y + gBlurRadius, gInput.Length.y-1);
		gCache[groupThreadID.y+2*gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
	}
	
	// Clamp out of bound samples that occur at image borders.
	gCache[groupThreadID.y+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];


	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	[unroll]
	for(int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.y + gBlurRadius + i;
		
		blurColor += gWeights[i+gBlurRadius]*gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}

technique11 HorzBlur
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, HorzBlurCS() ) );
    }
}

technique11 VertBlur
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, VertBlurCS() ) );
    }
}
