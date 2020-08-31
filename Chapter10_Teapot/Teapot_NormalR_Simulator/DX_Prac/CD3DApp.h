#pragma once
#include <d3d9.h>
#include <d3dx9math.h>
#include <d3dx9shape.h>

class CD3DApp
{
protected:
	LPDIRECT3D9 m_pD3D;
	LPDIRECT3DDEVICE9 m_pd3dDevices;
	HWND m_hWnd;

	virtual void OnInit() = 0;
	virtual void OnRender() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRelease() = 0;

public:
	CD3DApp(void);
	~CD3DApp(void);
	HRESULT D3DInit(HWND hWnd);
	void Render();
	void Update();
	void Cleanup();
};