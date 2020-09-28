#pragma once
#include <d3d9.h>

struct CUSTOMVERTEX // 정점 구조체 방법 1
{
	float x, y, z, rhw; // rhw는 2d 즉, 모니터 상의 좌표계를 통해서 정점을 나태 낼 때에 쓰인다.
	DWORD color;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE) // 정점 출력 모드 묶기

class CTriangle
{
private:
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DDEVICE9 m_pd3dDevice;

public:
	void OnInit(LPDIRECT3DDEVICE9 pd3dDevice);
	void OnRender();
	void OnUpdate();
	void OnRelease();

public:
	CTriangle();
	~CTriangle();
};

