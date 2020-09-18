#pragma once
#include "CD3DApp.h"

struct CUSTOMVERTEX // 정점 형식, 정점 색상은 빛 반사에 영향을 주기에 빼버렸다.
{
	D3DXVECTOR3 vPos;
	D3DXVECTOR3 vNormal; // 법선 벡터
};

struct CROSSVERTEX // 법선 벡터용 정점 형식
{
	D3DXVECTOR3 vPos;
	DWORD dwDiffuse;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)
#define D3DFVF_CROSSVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

class CGameEdu01 : public CD3DApp
{
	D3DXMATRIX m_matView;
	D3DXMATRIX m_matProj; // 투영 변환을 위한 행렬
	D3DXVECTOR3 m_Eye, m_At, m_Up; // 뷰 변환을 위한 위치 (카메라 위치, 보는 시점)

	// 정점 종류가 두개라 정점 버퍼도 두 개다.
	LPDIRECT3DVERTEXBUFFER9 m_pVB; // 삼각형(지면) 그리는 정점 버퍼
	LPDIRECT3DVERTEXBUFFER9 m_pCrossVB; // 법선 정점 버퍼

private:
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();

public:
	// 광원 조절을 위한 API 변수, 함수들
	D3DMATERIAL9 m_Material; // 물질 처리 구조체
	D3DLIGHT9 m_Light; // 광원 처리 구조체
	void SetMaterial(float r, float g, float b);
	void SetDirectionalLight();

public:
	CGameEdu01(void);
	~CGameEdu01(void);
};

