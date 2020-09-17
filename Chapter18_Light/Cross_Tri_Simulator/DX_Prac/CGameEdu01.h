#pragma once
#include "CD3DApp.h"

struct CUSTOMVERTEX // 삼각형 정점의 형식
{
	D3DXVECTOR3 vPos;
	D3DXVECTOR3 vNormal;
	DWORD dwDiffuse; // 색상
};

struct CROSSVERTEX // 법선 정점의 형식
{
	D3DXVECTOR3 vPos;
	DWORD dwDiffuse;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE)
#define D3DFVF_CROSSVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

class CGameEdu01 : public CD3DApp
{
	D3DXMATRIX m_matView;
	D3DXMATRIX m_matProj; // 투영 변환을 위한 행렬
	D3DXVECTOR3 m_Eye, m_At, m_Up; // 뷰 변환을 위한 위치 (카메라 위치, 보는 시점)

	// 정점 종류가 두개라 정점 버퍼도 두 개다.
	LPDIRECT3DVERTEXBUFFER9 m_pVB; // 삼각형 그리는 정점 버퍼
	LPDIRECT3DVERTEXBUFFER9 m_pCrossVB; // 법선들 그리는 정점 버퍼

private:
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();

public:
	CGameEdu01(void);
	~CGameEdu01(void);
};

