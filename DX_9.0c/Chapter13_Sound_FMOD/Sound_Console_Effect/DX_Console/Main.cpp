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

FMOD_SYSTEM* g_System; // FMOD 사운드 시스템을 이용하기 위한 포인터 객체
FMOD_SOUND* g_Sound[2]; // 포인터 배열임을 주의!
FMOD_CHANNEL* g_Channel; // 배경음 출력

void FMOD_Init() // FMOD 시스템 초기화 함수
{
	// FMOD 초기화 및 사운드 파일 로딩
	FMOD_System_Create(&g_System);
	FMOD_System_Init(g_System, 32, FMOD_INIT_NORMAL, NULL); // FMOD_INIT_NORMAL은 일반 효과음에 쓰이는 모드
	// 실제 소리파일을 각 g_Sound와 1:1 대응시킴, 반복은 FMOD_LOOP_NORMAL, 기본 효과음은 FMOD_DEFAULT 이용 
	FMOD_System_CreateSound(g_System, "Sound_Dir\\wave.mp3", FMOD_LOOP_NORMAL, 0, &g_Sound[0]);
	FMOD_System_CreateSound(g_System, "Sound_Dir\\safe.wav", FMOD_DEFAULT, 0, &g_Sound[1]);

	// 배경음 출력
	FMOD_System_PlaySound(g_System, g_Sound[0], 0, 0, &g_Channel);
	FMOD_Channel_SetVolume(g_Channel, 0.3f);
}

void FMOD_Release() // FMOD 메모리 해제 함수
{
	// Note: FMOD 해제
	int i;

	for (i = 0; i < 2; i++)
		FMOD_Sound_Release(g_Sound[i]); // FMOD sound 객체 해제

	FMOD_System_Close(g_System); // FMOD system 객체 clsoe
	FMOD_System_Release(g_System); // FMOD system 객체 해제
}

int main()
{
	int nKey;
	FMOD_CHANNEL* pChannel;
	float volume = 0.5f;

	FMOD_Init();

	printf("s를 눌러 효과음 출력\n");

	while (1)
	{
		if (_kbhit())
		{
			nKey = _getch();

			if (nKey == 's') // s 누르면 효과음 출력
			{
				FMOD_System_PlaySound(g_System, g_Sound[1], 0, 0, &pChannel);
				FMOD_Channel_SetVolume(pChannel, volume);
			}
		}

		FMOD_System_Update(g_System);
	}

	FMOD_Release();
	return 0;
}