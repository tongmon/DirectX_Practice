#pragma once
#include <d3dx9.h>

class CGround
{
	DWORD m_dwNumVertices, m_dwNumIndices; // 정점과 인덱스의 개수
	LPDIRECT3DINDEXBUFFER9	m_pIB; // 인덱스 버퍼 포인터
	LPDIRECT3DVERTEXBUFFER9	m_pVB; // 정점 버퍼 포인터
	LPDIRECT3DDEVICE9 m_pd3dDevice; // 디바이스 포인터
	int m_nCol, m_nRow; // 맵 좌표의 행렬 크기

public:
	void Create(LPDIRECT3DDEVICE9 pd3dDevice, int nRow, int nCol, float fSize);
	void OnRender();
	void OnRelease();

public:
	CGround(void);
	~CGround(void);
};

