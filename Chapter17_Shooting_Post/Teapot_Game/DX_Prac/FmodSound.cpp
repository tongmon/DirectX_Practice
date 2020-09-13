#include "FmodSound.h"

CFmodSound::CFmodSound(void)
{
	FMOD_System_Create(&m_pSystem);
	FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);
}

CFmodSound::~CFmodSound(void) // 시스템 해제
{
	FMOD_System_Close(m_pSystem);
	FMOD_System_Release(m_pSystem);
}

void CFmodSound::CreateBGSound(int nCount, string* SoundFileName) // 배경음 소리 파일 이름 쭈루루룩 읽어서 배열에 삽입
{
	// 효과 사운드 생성
	m_nBGSoundCount = nCount;
	m_ppBGSound = new FMOD_SOUND * [nCount];
	m_ppBGChannel = new FMOD_CHANNEL * [nCount]; // 배경음은 중간에 멈춰야 하는 경우도 있어서 채널도 배열로 생성

	for (int i = 0; i < nCount; i++) // 하나씩 배열에 삽입해준다.
	{
		FMOD_System_CreateSound(m_pSystem, SoundFileName[i].data(), FMOD_LOOP_NORMAL, 0, &m_ppBGSound[i]);
	}
}

void CFmodSound::CreateEffectSound(int nCount, string* SoundFileName) // 얘는 효과음
{
	// 백 그라운드 사운드 생성
	m_nEFSoundCount = nCount;
	m_ppEFFSound = new FMOD_SOUND * [nCount];

	for (int i = 0; i < nCount; i++)
	{
		FMOD_System_CreateSound(m_pSystem, SoundFileName[i].data(), FMOD_DEFAULT, 0, &m_ppEFFSound[i]);
	}
}

void CFmodSound::PlaySoundEffect(int nIndex) // 효과음 재생
{
	if (nIndex < m_nEFSoundCount)
	{
		FMOD_CHANNEL* pChannel; // 효과음은 어짜피 짧아서 중간에 멈출 필요가 없어 로컬로 채널 생성
		FMOD_System_PlaySound(m_pSystem, m_ppEFFSound[nIndex], nullptr, false, &pChannel);
	}
}

void CFmodSound::PlaySoundBG(int nIndex) // 배경음 재생
{
	if (nIndex < m_nBGSoundCount)
	{
		FMOD_System_PlaySound(m_pSystem, m_ppBGSound[nIndex], nullptr, false, &m_ppBGChannel[nIndex]);
	}
}

void CFmodSound::StopSoundBG(int nIndex)
{
	if (nIndex < m_nBGSoundCount)
	{
		FMOD_Channel_Stop(m_ppBGChannel[nIndex]);
	}
}

void CFmodSound::Update() // 소리 업데이트
{
	// 업데이트를 하는 이유는 소리 파일 전체를 메모리에 올리는건 비효율적이라 부분 부분 짤라서 올리는데 이 부분 부분을 짤라 가져오기 위해 해준다.
	if (!m_pSystem)
	{
		FMOD_System_Update(m_pSystem);
	}
}

void CFmodSound::ReleaseSound()
{
	int i;

	delete[] m_ppBGChannel;

	for (i = 0; i < m_nBGSoundCount; i++)
	{
		FMOD_Sound_Release(m_ppBGSound[i]);
	}
	delete[] m_ppBGSound;
	for (i = 0; i < m_nEFSoundCount; i++)
	{
		FMOD_Sound_Release(m_ppEFFSound[i]);
	}
	delete[] m_ppEFFSound;
}