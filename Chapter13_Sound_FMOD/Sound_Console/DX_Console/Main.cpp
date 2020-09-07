#include <iostream>
#include <conio.h>
#include <d3d9.h>
#include <d3dx9math.h>
#include <fmod.h>
using namespace std;

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"DxErr.lib")
#pragma comment (lib,"d3dxof.lib")
#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"fmod_vc.lib")
#pragma comment (lib,"fmodL_vc.lib")

/*
코드가 약간 구형이라 헷갈릴 수 있으니 https://m.blog.naver.com/PostView.nhn?blogId=zoomen1004&logNo=110175532047&proxyReferer=https:%2F%2Fwww.google.com%2F
위 홈페이지 참고
*/

FMOD_SYSTEM* g_System;  // Note: FMOD system 포인터 변수 선언, 다렉처럼 기기 포인터를 통해서 소리 출력한다
FMOD_SOUND* g_Sound;   // Note: FMOD Sound 포인터 변수 선언, 소리 파일과 1:1 대응한다.

void FMOD_Init() // FMOD 시스템 초기화 함수
{
	// FMOD 초기화 및 사운드 파일 로딩
	FMOD_System_Create(&g_System);
	FMOD_System_Init(g_System, 32, FMOD_INIT_NORMAL, NULL);
	FMOD_System_CreateSound(g_System, "Sound_Dir\\wave.mp3", FMOD_DEFAULT, 0, &g_Sound); // FMOD_DEFAULT는 한번만 재생하고 꺼진다. 반복이 안됨
}

void FMOD_Release() // FMOD 메모리 해제 함수
{
	// Note: FMOD 해제
	FMOD_Sound_Release(g_Sound);
	FMOD_System_Close(g_System);
	FMOD_System_Release(g_System);
}

int main()
{
	int nKey;
	FMOD_CHANNEL* channel = NULL; // 채널은 한 소리당 여러개 배정가능, 보통 소리 중단할 때 많이 쓰임
	float volume = 0.3f; // 볼륨 범위는 0.0f ~ 1.0f 까지

	FMOD_Init(); // 초기화 및 SYSTEM 기기할당

	printf("----------------------\n");
	printf("☞ MUSIC PLAYER ☜ \n");
	printf("p : 시작 \n");
	printf("s : 중지 \n");
	printf("u : 볼륨 증가 \n");
	printf("d : 볼륨 감소 \n");
	printf("q : 종료 \n");
	printf("----------------------\n");

	while (1)
	{
		if (_kbhit())  // 키 입력 감지   
		{
			nKey = _getch();   // 입력된 키 값을 키보드 버퍼로부터 가져옴
			if (nKey == 'q')  // q 키가 눌리면 프로그램 종료, 키 입력중 가장 최우선 순위가 된다. 
			{
				break;
			}

			switch (nKey)
			{
			case 'p':
				FMOD_System_PlaySound(g_System, g_Sound, 0, 0, &channel);
				FMOD_Channel_SetVolume(channel, volume);
				break;
			case 's':
				FMOD_Channel_Stop(channel);
				break;
			case 'u': // 볼륨 업
				volume += 0.1f;
				if (volume > 1.0f)
				{
					volume = 1.0f;
				}
				FMOD_Channel_SetVolume(channel, volume);
				break;
			case 'd': // 볼륨 다운
				volume -= 0.1f;
				if (volume < 0.0f)
				{
					volume = 0.0f;
				}
				FMOD_Channel_SetVolume(channel, volume);
				break;
			}
		}
		FMOD_System_Update(g_System); // FMOD 업데이트, 업데이트를 해주는 이유는 스트리밍 방식으로 부분 부분 잘라서 메모리에 올려놓기 때문.
		// 틀어놓는 음악 전체를 한번에 메모리에 탑재하지 않는다. (비효율적)
	}

	FMOD_Release(); // 해제
	return 0;
}