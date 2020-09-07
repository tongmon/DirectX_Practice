#include "CGameEdu01.h"

CGameEdu01::CGameEdu01()
{
	m_cordX = m_cordY = m_cordZ = 0.0f;
}

CGameEdu01::~CGameEdu01()
{

}

void CGameEdu01::OnInit()
{
	RECT rect;
	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;

	// 뷰 크기 설정 (창 크기로 일단 설정 해놓고 창 크기 바뀔때마다 다시 설정)
	GetClientRect(m_hWnd, &rect);
	vp.Width = rect.right - rect.left; // 좌표 개념이 아니라 크기 개념
	vp.Height = rect.bottom - rect.top;

	// 깊이 버퍼의 범위는 0 ~ 1 사이
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;

	m_Eye.x = 0.0f;
	m_Eye.y = 80.0f;
	m_Eye.z = -0.00001f;

	m_At.x = 0.0f; m_At.y = 0.0f; m_At.z = 0.0f;

	m_Up.x = 0.0f; m_Up.y = 1.0f; m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 뷰 변환 행렬 생성
	m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f); // 프로젝션 변환 행렬 생성 (투영)
	m_pd3dDevices->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevices->SetViewport(&vp); // 뷰 포트 설정

	D3DXCreateFont(m_pd3dDevices, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		"System", &m_pFont); // 폰트 생성 1

	D3DXCreateFont(m_pd3dDevices, 30, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		"돋움체", &m_pFont2); // 폰트 생성 2

	D3DXCreateSphere(m_pd3dDevices, 3.0f, 30, 10, &m_Circle_Coltest, NULL); // 구

	m_Axis.OnInit(m_pd3dDevices);
}

void CGameEdu01::OnRender()
{
	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE); // 라이트 켜주면 안보이기에 일단 꺼줌
	m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME); // 뼈대만 보이기
	m_pd3dDevices->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // 컬링 제거, 뒷면 보기 가능

	m_Axis.OnRender();
	
	char string[100];
	RECT rt; // 글자를 위치시키기 위한 사각형

	SetRect(&rt, 10, 10, 0, 0);
	wsprintf(string, "FPS: %d", m_nFPS);
	m_pFont->DrawText(NULL, string, -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f)); // 색상은 RGB 그리고 투명도가 위치한다.
	
	m_nFPSCount++; // 1초가 되기 전까지 FPS를 계속 증가시켜 준다.


	D3DXMatrixTranslation(&m_Movement, m_cordX, m_cordY, m_cordZ);
	m_pd3dDevices->SetTransform(D3DTS_WORLD, &m_Movement); // 월드, 즉 물체 좌표를 스케일링해서 위치시킨다.
	m_Circle_Coltest->DrawSubset(0);
	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, TRUE); // 라이트 켜주면 안보이기에 일단 꺼줌
}

void CGameEdu01::OnUpdate()
{
	DWORD dwCurTime = GetTickCount64(); // 현재 시각을 따온다
	static DWORD dwOldTime = GetTickCount64(); // 정적 변수로 현재 시각을 한번만 초기화하고 그 다음 호출시 예전 시간이 된다.
	static DWORD dwAccumulateTime = 0; // 누적 시간
	m_dwElapsedTime = dwCurTime - dwOldTime; // 현재 시각과 예전 시각의 차이를 지난 시간에 넣어준다.
	dwOldTime = dwCurTime; // 현재 시각은 다음 호출시 예전 시각이 된다.

	dwAccumulateTime = dwAccumulateTime + m_dwElapsedTime; // 누적 시간은 지난 시간이 쌓여간다.
	if (dwAccumulateTime >= 1000) // 1초가 되었으면 FPS 출력
	{
		dwAccumulateTime = 0; // 누적 시간 초기화
		m_nFPS = m_nFPSCount; // 현재 까지 증가시킨 FPS 값을 화면에 출력하기 위해 m_nFPS에 넣어준다.
		m_nFPSCount = 0; // 다시 FPS 초기화
	}

	if (GetAsyncKeyState('A') < 0)
	{
		m_cordX = m_cordX - 1.0f;
	}
	if (GetAsyncKeyState('D') < 0)
	{
		m_cordX = m_cordX + 1.0f;
	}
	if (GetAsyncKeyState('W') < 0)
	{
		m_cordZ = m_cordZ + 1.0f;
	}
	if (GetAsyncKeyState('S') < 0)
	{
		m_cordZ = m_cordZ - 1.0f;
	}
}

void CGameEdu01::OnRelease()
{
	m_pFont->Release();
	m_pFont2->Release();

	m_Axis.OnRelease();
	m_Circle_Coltest->Release();
}