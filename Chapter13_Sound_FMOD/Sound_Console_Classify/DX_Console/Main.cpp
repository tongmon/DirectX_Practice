#include <iostream>
#include <conio.h>
#include <d3d9.h>
#include <d3dx9math.h>
#include <fmod.h>
#include "FmodSound.h"
using namespace std;

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"DxErr.lib")
#pragma comment (lib,"d3dxof.lib")
#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"fmod_vc.lib")
#pragma comment (lib,"fmodL_vc.lib")

int main()
{
	string strBGFileName[] = { "Sound_Dir\\story.mp3", "Sound_Dir\\wave.mp3" };
	string strEffectFileName[] = { "Sound_Dir\\play_fire.wav", "Sound_Dir\\safe.wav" };
	int nKey;

	CFmodSound FmodSound; // 소리 객체 생성
	FmodSound.CreateBGSound(2, strBGFileName); // 배경음 소리를 넣어준다.
	FmodSound.CreateEffectSound(2, strEffectFileName); // 효과음 소리를 넣어준다.

	printf("사운드 시스템 준비 완료\n");
	printf("키 1: 배경1 출력	키 2: 효과음 출력	키 3: 배경음 중지	키 q: 종료\n");

	while (true)
	{
		if (_kbhit())
		{
			nKey = _getch();
			if (nKey == '1')
			{
				FmodSound.PlaySoundBG(0);
			}
			if (nKey == '2')
			{
				FmodSound.PlaySoundEffect(1);
			}
			if (nKey == '3')
			{
				FmodSound.StopSoundBG(0);
			}
			if (nKey == 'q')
			{
				break;
			}
		}
		FmodSound.Update();
	}
	FmodSound.ReleaseSound();
	return 0;
}