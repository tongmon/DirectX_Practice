#pragma once
#include "CD3DApp.h"

struct CUSTOMVERTEX
{
	D3DXVECTOR3 vPos;
	float tu, tv; // tu, tv는 그림과 대응되는 정점 값이다. 시작점 (0,0) ~ (1,1) 까지 대응된다.
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

class CGameEdu01 : public CD3DApp
{
	D3DXMATRIX m_matView;
	D3DXMATRIX m_matProj; // 투영 변환을 위한 행렬
	D3DXVECTOR3 m_Eye, m_At, m_Up; // 뷰 변환을 위한 위치 (카메라 위치, 보는 시점)

	// 정점 버퍼들
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DTEXTURE9 m_pTexture; // 텍스쳐에 대응되는 정점 버퍼

	D3DXMATRIX m_matScale[2], m_matTranslate[2];

private:
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();

public:
	CGameEdu01(void);
	~CGameEdu01(void);
};

