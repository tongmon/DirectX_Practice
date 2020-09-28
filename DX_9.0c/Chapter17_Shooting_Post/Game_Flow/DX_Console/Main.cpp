#include <windows.h>
#include <iostream>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <d3d9.h>
#include <d3dx9math.h>
#include "Screen.h" // 콘솔 화면 출력을 위한 함수들이 위치
using namespace std;

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"DxErr.lib")
#pragma comment (lib,"d3dxof.lib")
#pragma comment (lib,"winmm.lib")

typedef enum _GAME_STATE { INIT, READY, RUNNING, STOP, SUCCESS, FAILED, RESULT } GAME_STATE;

unsigned short g_GameState = INIT; // 게임 흐름이 어느 정도인지 알기 위한 척도 변수, 처음은 무조건 초기화니 INIT 부분이다.
int g_nGoal, g_nStage, g_GameStartTime;

char g_strMessage[100]; // 게임 상태를 저장하는 문자열
clock_t g_OldTime; // 게임 상태 전이를 위한 이전 시각 저장

void Init()
{
	g_OldTime = clock();
}

void Update()
{
	clock_t CurTime = clock();

	switch (g_GameState)
	{
	case INIT: // 초기화 부분
		if (g_nStage == 0)
		{
			wsprintf(g_strMessage, "%s", "게임 및 사운드 초기화");
			if (CurTime - g_OldTime > 3000) // 3초 이후에 초기화가 모두 끝났다고 친다.
			{
				g_OldTime = CurTime;
				g_nStage = 1;
			}
		}
		else 
		{
			wsprintf(g_strMessage, "[INIT] 게임 %d 스테이지 초기화", g_nStage);
			if (CurTime - g_OldTime > 3000)
			{
				g_OldTime = CurTime; // 다음 단계로 넘어갈때 OldTime 변경, 각 단계별 3초를 보여주어야 하기 때문
				g_GameState = READY; // 초기화가 끝났으니 준비 단계로 넘어간다.
			}
		}
		break;
	case READY: // 시작 준비 부분
		wsprintf(g_strMessage, "[READY] %d 스테이지", g_nStage);
		if (CurTime - g_OldTime > 3000) // 각 단계별 3초가 지난 후에 다음 단계로 진행
		{
			g_OldTime = CurTime;
			g_GameState = RUNNING; // 준비가 끝났으니 RUNNING으로 넘어간다.
			g_GameStartTime = CurTime; // 게임 시작을 했으니 시작 시간 기록
		}
		break;
	case RUNNING: // 게임 시작 및 진행
		if (CurTime - g_GameStartTime > 10000) // Note: 제한 시간
		{
			g_GameState = STOP; // 10초가 지나면 STOP 단계로 넘어간다.
		}
		else 
		{
			wsprintf(g_strMessage, "[RUNNING] 제한 시간 : 10초  현재시간 : %d", (CurTime - g_GameStartTime) / 1000);
		}
		break;
	case STOP: // 게임 멈춤 상태
		if (g_nGoal == 1) // 골을 넣은 상태로 STOP 상태에 왔다면 게임의 상태를 SUCCESS로 넘긴다.
		{
			g_GameState = SUCCESS;
		}
		else // 골을 못넣었다면 FAILED 상태로 넘긴다.
		{
			g_GameState = FAILED;
		}
		break;
	case SUCCESS: // 성공 상태
		wsprintf(g_strMessage, "%s", "미션 성공");
		if (CurTime - g_OldTime > 3000)
		{
			g_OldTime = CurTime;
			g_GameState = INIT;
			++g_nStage;
		}
		break;
	case FAILED: // 실패 상태
		wsprintf(g_strMessage, "%s", "미션 실패! 계속 하시겠습니까? <y/n> ");
		break;

	case RESULT: // 결과 상태
		wsprintf(g_strMessage, "%s", "게임 결과 화면");
		if (CurTime - g_OldTime > 3000)
		{
			g_OldTime = CurTime;
		}
		break;
	}
}

void Render() // 렌더 부분, 실제로는 렌더도 각 단계별로 나누어져야 하지만 여기선 글자 하나만 출력하기에 나누진 않았다.
{
	clock_t CurTime = clock();

	ScreenClear();

	// Note: 렌더링 시작	
	ScreenPrint(20, 10, g_strMessage);

	// Note: 렌더링 끝 
	ScreenFlipping();
}

void Release()
{
}

int main(void)
{
	int nKey;

	ScreenInit();
	Init(); // 초기화

	while (true)
	{
		if (g_GameState == RESULT) // 결과 화면이면 끝
		{
			return 1;
		}

		if (_kbhit())
		{
			nKey = _getch();

			if (nKey == 'q') // q로 나가기
				break;

			switch (nKey)
			{
			case 's':
				g_nGoal = 1;
				break;
			case 'f':
				g_nGoal = 0;
				break;

			// y, Y 이면 수락 
			case 'y':
			case 'Y':
				if (g_GameState == FAILED) // 실패하면
				{
					g_GameState = INIT; // 수락하여 다시 시작
					g_OldTime = clock();
				}
				break;

			// n, N 이면 수락 
			case 'n':
			case 'N':
				if (g_GameState == FAILED) // 실패 상태에서 거부하면 결과 상태로 이동
				{
					g_GameState = RESULT;
					g_OldTime = clock();
				}
				break;
			}
		}

		Update();    // 데이터 갱신
		Render();    // 화면 출력		
	}

	Release();   // 해제
	ScreenRelease();
	return 0;
}
