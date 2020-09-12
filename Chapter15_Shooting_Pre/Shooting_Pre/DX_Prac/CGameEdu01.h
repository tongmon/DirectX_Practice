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

