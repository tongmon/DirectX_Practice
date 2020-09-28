#pragma once
#include "CD3DApp.h"
#include "CAxis.h"
#include "CCube.h"
#include "CGround.h"

class CGameEdu01 : public CD3DApp
{
	D3DXMATRIX m_matView;
	D3DXMATRIX m_matProj; // 투영 변환을 위한 행렬
	D3DXVECTOR3 m_Eye, m_At, m_Up; // 뷰 변환을 위한 위치 (카메라 위치, 보는 시점)
	CAxis m_Axis; // 화면에 출력할 축을 선언
	CCube m_Cube; // 화면에 출력할 큐브을 선언

	// 폰트
	LPD3DXFONT m_pFont;
	LPD3DXFONT m_pFont2;

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

