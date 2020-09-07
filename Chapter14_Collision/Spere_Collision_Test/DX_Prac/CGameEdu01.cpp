#include "CGameEdu01.h"

BOOL CheckSphereIntersect(D3DXVECTOR3 vCenter1, float fRadius1,
	D3DXVECTOR3  vCenter2, float fRadius2) // 원 위치에 따른 충돌체크 판단 함수, 간단히 중심과 반지름을 이용하면 구할 수 있다.
{
	float fDistance;
	D3DXVECTOR3 vDiff;

	vDiff = vCenter2 - vCenter1;
	fDistance = D3DXVec3Length(&vDiff);

	if (fDistance <= (fRadius1 + fRadius2))
	{
		return TRUE;  // 충돌
	}
	return FALSE; // 비 충돌
}

CGameEdu01::CGameEdu01()
{
	
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
	m_Eye.y = 5.0f;
	m_Eye.z = -20.0f;

	m_At.x = 0.0f; m_At.y = 0.0f; m_At.z = 0.0f;

	m_Up.x = 0.0f; m_Up.y = 1.0f; m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 뷰 변환 행렬 생성
	m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f); // 프로젝션 변환 행렬 생성 (투영)
	m_pd3dDevices->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevices->SetViewport(&vp); // 뷰 포트 설정

	D3DXCreateFont(m_pd3dDevices, 15, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		"돋움체", &m_pFont); // 폰트 생성 1

	D3DXCreateSphere(m_pd3dDevices, 1.0f, 20, 20, &m_pMesh, NULL); // 구 생성

	// 구_1 속성 초기화
	m_Sphere[0].fRadius = 1.0f;
	m_Sphere[0].fScaling = 1.0f;
	m_Sphere[0].vTans = D3DXVECTOR3(0, 0, 0);
	// 구_2 속성 초기화
	m_Sphere[1].fRadius = 1.0f;
	m_Sphere[1].fScaling = 2.0f;
	m_Sphere[1].vTans = D3DXVECTOR3(5, 0, 0);

	m_bIsCollision = FALSE; // 초반 충돌 X

	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
}

void CGameEdu01::OnRender()
{
	char string[100];
	RECT rt; // 글자를 위치시키기 위한 사각형
	D3DXMATRIX matWorld, matScale, matTrans;

	for (int i = 0; i < 2; i++) // 각 구들을 속성에 맞게 출력
	{
		D3DXMatrixTranslation(&matTrans, m_Sphere[i].vTans.x, m_Sphere[i].vTans.y, m_Sphere[i].vTans.z);
		D3DXMatrixScaling(&matScale, m_Sphere[i].fScaling, m_Sphere[i].fScaling, m_Sphere[i].fScaling);
		matWorld = matScale * matTrans;
		m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld);

		m_pMesh->DrawSubset(0);
	}

	SetRect(&rt, 10, 10, 0, 0);
	wsprintf(string, "FPS: %d", m_nFPS);
	m_pFont->DrawText(NULL, string, -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f)); // 색상은 RGB 그리고 투명도가 위치한다.
	
	m_nFPSCount++; // 1초가 되기 전까지 FPS를 계속 증가시켜 준다.

	SetRect(&rt, 10, 32, 0, 0);
	if (m_bIsCollision)
	{
		m_pFont->DrawText(NULL, "충돌", -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	}
	else
	{
		m_pFont->DrawText(NULL, "비충돌", -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	}
}

void CGameEdu01::OnUpdate()
{
	DWORD dwCurTime = GetTickCount64(); // 현재 시각을 따온다
	static DWORD dwOldTime = GetTickCount64(); // 정적 변수로 현재 시각을 한번만 초기화하고 그 다음 호출시 예전 시간이 된다.
	static DWORD dwAccumulateTime = 0; // 누적 시간
	m_dwElapsedTime = dwCurTime - dwOldTime; // 현재 시각과 예전 시각의 차이를 지난 시간에 넣어준다. (한 프레임)
	dwOldTime = dwCurTime; // 현재 시각은 다음 호출시 예전 시각이 된다.

	dwAccumulateTime = dwAccumulateTime + m_dwElapsedTime; // 누적 시간은 지난 시간이 쌓여간다.
	if (dwAccumulateTime >= 1000) // 1초가 되었으면 FPS 출력
	{
		dwAccumulateTime = 0; // 누적 시간 초기화
		m_nFPS = m_nFPSCount; // 현재 까지 증가시킨 FPS 값을 화면에 출력하기 위해 m_nFPS에 넣어준다.
		m_nFPSCount = 0; // 다시 FPS 초기화
	}

	// 델타타임으로 움직임
	if (GetAsyncKeyState(VK_LEFT))
	{
		m_Sphere[1].vTans.x -= m_dwElapsedTime * 0.003f; // m_dwElapsedTime(시간), 0.003f(속도) -> 시간 x 속도 = 거리
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		m_Sphere[1].vTans.x += m_dwElapsedTime * 0.003f;
	}
	m_bIsCollision = CheckSphereIntersect(m_Sphere[0].vTans, m_Sphere[0].fRadius * m_Sphere[0].fScaling,
		m_Sphere[1].vTans, m_Sphere[1].fRadius * m_Sphere[1].fScaling); // 충돌 체크
}

void CGameEdu01::OnRelease()
{
	m_pFont->Release();
	m_pMesh->Release();
}