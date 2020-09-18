#pragma once
#include <d3d9.h>
#include <d3dx9math.h>

struct CUSTOMVERTEX
{
	FLOAT x, y, z; // The transformed position for the vertex.
	DWORD color;        // The vertex color.
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE) // x,y,z 좌표와 색깔만을 설정

class CAxis
{
private:
	LPDIRECT3DDEVICE9   m_pd3dDevice; // 디바이스 변경해야 하니까 디바이스 포인터
	LPDIRECT3DVERTEXBUFFER9 m_pVB; // 정점을 담아야 하니까 정점 버퍼 포인터

public:
	void OnInit(LPDIRECT3DDEVICE9 pd3dDevice);
	void OnRender();
	void OnRelease();

public:
	CAxis(void);
	~CAxis(void);
};
