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

	LPD3DXMESH m_pMesh; // 정점 정보들이 담기게 되는 메쉬 자료형
	D3DMATERIAL9* m_pMeshMaterials; // .x 파일 안에는 여러개의 머터리얼이 존재한다.
	LPDIRECT3DTEXTURE9* m_pMeshTextures; // .x 파일 안에는 머터리얼과 대응하는 그 만큼의 텍스쳐 또한 존재한다.
	DWORD m_dwNumMaterials; // 머터리얼과 텍스쳐의 개수(배열의 인덱스 크기로 작용)

private:
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();

public:
	CGameEdu01(void);
	~CGameEdu01(void);
};

