#pragma once
#include <string>
#include <fmod.h>
using namespace std;

class CFmodSound
{
private:
	FMOD_SYSTEM* m_pSystem; // FMOD 시스템
	FMOD_SOUND** m_ppBGSound; // 이중포인터로 한 이유는 소리가 여러개 일 수 있으니 포인터 배열을 만들어 저장하려고 사용했다.
	FMOD_SOUND** m_ppEFFSound; // 백그라운드 사운드와 같은 이유로 효과음 사운드도 이중포인터 선언
	FMOD_CHANNEL** m_ppBGChannel; // 채널도 물론 여러개니 이중 포인터
	int m_nEFSoundCount;
	int m_nBGSoundCount;
public:
	// string* 는 이차원 배열이라고 생각하면 쉽다. 걍 파일명 여러개 묶어서 편리하게 저장하는 용도
	void CreateEffectSound(int nCount, string* SoundFileName);
	void CreateBGSound(int nCount, string* SoundFileName);
	// 인덱스로 포인터 배열에서 소리 선택해 작동
	void PlaySoundEffect(int nIndex);
	void PlaySoundBG(int nIndex);
	void StopSoundBG(int nIndex); // 배경음은 멈춰야 할 때도 있기에 생성
	void ReleaseSound();
	void Update();

public:
	CFmodSound(void);
	~CFmodSound(void);
};
