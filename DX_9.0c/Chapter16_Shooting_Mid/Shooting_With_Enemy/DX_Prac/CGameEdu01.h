#pragma once
#include "CD3DApp.h"
#include "CAxis.h"
#include "CCube.h"
#include "CGround.h"

struct BULLET // 총알 자체에 대한 속성, 총알 한발마다 다른 값들이 위치한다.
{
	int nLife;
	D3DXVECTOR3 vPos;
	D3DXMATRIX matTranslation; // 이동 행렬
};

struct BULLET_PROPERTY // 총알 출력에 대한 속성, 모든 총알이 공유하게 되는 속성 값들이 위치한다.
{
	float fBulletVelcoty; // 총알 속력
	float fScale; // 총알 크기
	D3DXMATRIX matScale; // 크기 행렬
};

struct PLAYER // 주인공
{
	int nLife; // 플레이어 체력
	D3DXVECTOR3 vPos; // 위치
	float fScale; // 크기
	float fRotationY; // 회전을 위한 값

	float fVelocity; // 일정한 이동 속도

	// 총알에 대한 속성	
	DWORD dwBulletFireTime; // 총알 발사 간격
	DWORD dwOldBulletFireTime; // 총알이 나간 시간

	// 밑의 항목들은 굳이 Player에 없어도 되지만 프로그래머 성향에 따라 이렇게 넣어두고 계산하는 경우가 있다.
	D3DXMATRIX matWorld;
	D3DXMATRIX matScale;
	D3DXMATRIX matRotationY;
	D3DXMATRIX matTranslation;
};

struct ENEMY // 적 개체마다의 개별 정보가 위치한다.
{
	int nLife; // 생명
	DWORD dwAppearTime; // 등장시간, 오름차순으로 설정한다.
	D3DXVECTOR3 vPos; // 위치

	DWORD dwOldFireTime; // 총알 발사 이전 시간
	D3DXMATRIX matTranslation; // 적 이동 행렬
};

struct ENEMY_PROPERTY // 적들이 공유하게 되는 속성, 게임으로 치면 '종족' 이라고 볼 수 있다. 이 값들을 다양하게 함으로써 다양한 형태의 적들이 생성된다.
{
	float fMoveVelcoty; // 이동 속도
	float fScale; // 적 크기

	DWORD dwFireTime; // 미사일 발사 시간 간격
	D3DXMATRIX matScale; // 크기 변환 행렬
};

class CGameEdu01 : public CD3DApp
{
	D3DXMATRIX m_matView;
	D3DXMATRIX m_matProj; // 투영 변환을 위한 행렬
	D3DXVECTOR3 m_Eye, m_At, m_Up; // 뷰 변환을 위한 위치 (카메라 위치, 보는 시점)

	// 지면 출력
	CGround m_Ground;

	// 주인공 
	PLAYER m_sPlayer; // 플레이어 선언
	BULLET_PROPERTY m_sPlayerBulletProperty;
	BULLET m_sPlayerBullet[10]; // 한번에 열개까지 쏠 수 있다.

	LPD3DXMESH m_pTeapotMesh; // 주인공 (주전자 메쉬)	
	LPD3DXMESH m_pPlayerBulletMesh; // 주인공 총알 (원기둥 메쉬)
	LPD3DXMESH m_pEnemyBoxMesh; // 적이 되는 박스 메쉬
	LPD3DXMESH m_pEnemyBulletMesh; // 적의 총알 메쉬

	// 적 캐릭터 
	ENEMY_PROPERTY m_EnemyProperty; // 적의 종류는 여기선 간단하게 한개로 설정
	ENEMY m_Enemy[100]; //적 캐릭터 100개 
	BULLET_PROPERTY m_EnemyBulletProperty; // 총알 종류는 일단 한개로 설정
	BULLET m_EnemyBullet[100]; // 적 총알 개수에 대한 적당량
	int m_nEnemyIndex; // 각 적을 구분하는 기준은 인덱스이다. 결국 적들은 배열로 이루어져있기에 구별하기 위한 인덱스 변수가 필요하다.
	int m_nEnemyCount;

	// 스테이지 정보
	DWORD m_dwGameStartTime;
	DWORD m_dwGameElapsedTime;

	// 글자 출력 폰트
	LPD3DXFONT m_pFont;

	// 게임 시간에 대한 변수
	DWORD m_dwElapsedTime; // 지나간 시간
	unsigned int m_nFPS, m_nFPSCount; // 1초가 되었을 경우 출력되는 FPS, 1초 되기 전까지 카운트되는 FPS 값

private:
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();

public:
	CGameEdu01(void);
	~CGameEdu01(void);
};

