#pragma once
#include "CD3DApp.h"
#include "CAxis.h"
#include "CCube.h"

class CGameEdu01 : public CD3DApp
{
	D3DXMATRIX m_matView;
	D3DXMATRIX m_matProj; // 투영 변환을 위한 행렬
	D3DXVECTOR3 m_Eye, m_At, m_Up; // 뷰 변환을 위한 위치 (카메라 위치, 보는 시점)
	CAxis m_Axis; // 화면에 출력할 축을 선언
	CCube m_Cube; // 화면에 출력할 큐브을 선언

	// 기존 도형 출력과 다르게 인덱스 버퍼니 정점 버퍼니 필요없이 
	// 한 개의 함수로 출력이 가능해 따로 클래스 없이 멤버 변수만 선언
	LPD3DXMESH m_pTeapotMesh; // 주전자 메쉬 선언
	float m_fScale;

private:
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();

public:
	CGameEdu01(void);
	~CGameEdu01(void);
};

