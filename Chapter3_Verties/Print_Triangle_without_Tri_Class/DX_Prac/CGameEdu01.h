#pragma once
#include "CD3DApp.h"

struct CUSTOMVERTEX
{
	float x, y, z, rhw; // rhw는 2d 즉, 모니터 상의 좌표계를 통해서 정점을 나태 낼 때에 쓰인다.
	DWORD color;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE) // 정점 출력 모드 묶기, Format

class CGameEdu01 : public CD3DApp
{
private:
	LPDIRECT3DVERTEXBUFFER9 m_pVB;

private:
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();

public:
	CGameEdu01(void);
	~CGameEdu01(void);
};

