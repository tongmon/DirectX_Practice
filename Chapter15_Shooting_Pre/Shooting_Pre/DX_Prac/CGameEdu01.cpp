#include "CGameEdu01.h"

BOOL CheckCubeIntersection(D3DXVECTOR3* vMin1, D3DXVECTOR3* vMax1,
	D3DXVECTOR3* vMin2, D3DXVECTOR3* vMax2) // 큐브 충돌 체크 함수, 각 큐브의 최대점과 최소점을 비교하여 겹치는 부분이 있으면 충돌로 판단한다.
{
	if (vMin1->x <= vMax2->x && vMax1->x >= vMin2->x &&
		vMin1->y <= vMax2->y && vMax1->y >= vMin2->y &&
		vMin1->z <= vMax2->z && vMax1->z >= vMin2->z)
		return TRUE;
	return FALSE;
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

	// 카메라 위치
	m_Eye.x = 0.0f;
	m_Eye.y = 10.0f;
	m_Eye.z = -32.0f;

	m_At.x = 0.0f; m_At.y = 0.0f; m_At.z = 0.0f; // 쳐다보는 위치

	m_Up.x = 0.0f; m_Up.y = 1.0f; m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 뷰 변환 행렬 생성
	m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f); // 프로젝션 변환 행렬 생성 (투영)
	m_pd3dDevices->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevices->SetViewport(&vp); // 뷰 포트 설정

	// 주인공(주전자) 메쉬 생성
	D3DXCreateTeapot(m_pd3dDevices, &m_pTeapotMesh, NULL);
	// 총알 메쉬 생성, 메쉬는 총알이 많다고 많이 생성할 필요가 없고 하나만 생성하고 총알 속성 객체(BULLET)를 많이 생성하면 되는 것이다.
	D3DXCreateCylinder(m_pd3dDevices, 2.0f, 0.0f, 10.0f, 15, 10, &m_pPlayerBulletMesh, NULL);
	// 지면 생성
	m_Ground.Create(m_pd3dDevices, 20, 6, 2.0f);

	// Note: 주인공 설정
	m_sPlayer.nLife = 10; // 체력 10으로
	m_sPlayer.fScale = 0.7f; // 크기 기존 크기보다 약간 작게
	m_sPlayer.fRotationY = -D3DXToRadian(90); // 반대 90도로 회전
	m_sPlayer.vPos = D3DXVECTOR3(0.0f, 0.0f, -9 * 2.0f); // 초기 위치 설정(카메라에 따라 z를 뒤로 빼준다.)
	m_sPlayer.fVelocity = 0.005f; // 속도 설정
	m_sPlayer.dwBulletFireTime = 400; // 총알 쏘는 간격
	m_sPlayer.dwOldBulletFireTime = GetTickCount64(); // 총알 쏜 시간, 그냥 초기화라고 생각하면 됨

	D3DXMatrixScaling(&m_sPlayer.matScale, m_sPlayer.fScale, m_sPlayer.fScale, m_sPlayer.fScale); // 캐릭터의 스케일링 행렬 생성
	D3DXMatrixTranslation(&m_sPlayer.matTranslation, m_sPlayer.vPos.x, m_sPlayer.vPos.y, m_sPlayer.vPos.z); // 캐릭터의 이동 행렬 생성
	D3DXMatrixRotationY(&m_sPlayer.matRotationY, m_sPlayer.fRotationY); // 캐릭터의 회전 행렬 생성

	// Note: 주인공 총알 설정
	m_sPlayerBulletProperty.fBulletVelcoty = 0.01f;
	m_sPlayerBulletProperty.fScale = 0.08f;
	D3DXMatrixScaling(&m_sPlayerBulletProperty.matScale, m_sPlayerBulletProperty.fScale, m_sPlayerBulletProperty.fScale, m_sPlayerBulletProperty.fScale); // 총알 스케일일 행렬

	// 폰트 생성
	D3DXCreateFont(m_pd3dDevices, 15, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &m_pFont);
}

void CGameEdu01::OnRender()
{
	char string[100];
	RECT rt; // 글자를 위치시키기 위한 사각형
	D3DXMATRIX matWorld; // 스케일, 위치, 회전 변환을 모아 캐릭터를 변환하게 될 행렬 (SRT)
	m_Ground.OnRender(); // 지면 출력

	// 라이팅은 일단 꺼놓고 뼈대만 보이게 출력한다.
	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	// 주인공 미사일
	for (int i = 0; i < 10; i++)
	{
		if (m_sPlayerBullet[i].nLife > 0) // 총알 체력이 1 이상이면 속성값대로 출력해준다.
		{
			matWorld = m_sPlayerBulletProperty.matScale * m_sPlayerBullet[i].matTranslation; // 총알은 회전이 필요없어 ST만 해준다.
			m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld); // 총알을 월드에 변환한 상태로 가져다 놓음
			m_pPlayerBulletMesh->DrawSubset(0);
		}
	}

	// 주인공 출력		
	m_pd3dDevices->SetTransform(D3DTS_WORLD, &m_sPlayer.matWorld);
	m_pTeapotMesh->DrawSubset(0);

	// FPS 문구 출력
	SetRect(&rt, 10, 10, 0, 0);
	wsprintf(string, "FPS: %d", m_nFPS);
	m_pFont->DrawText(NULL, string, -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f)); // 색상은 RGB 그리고 투명도가 위치한다.
	
	m_nFPSCount++; // 1초가 되기 전까지 FPS를 계속 증가시켜 준다.
}

void CGameEdu01::OnUpdate()
{
	int i; // 반복문을 위한 변수
	DWORD dwCurTime = GetTickCount64(); // 현재 시각을 따온다
	static DWORD dwOldTime = GetTickCount64(); // 정적 변수로 현재 시각을 한번만 초기화하고 그 다음 호출시 예전 시간이 된다.
	static DWORD dwAccumulateTime = 0; // 누적 시간
	m_dwElapsedTime = dwCurTime - dwOldTime; // 현재 시각과 예전 시각의 차이를 지난 시간에 넣어준다. (한 프레임)
	dwOldTime = dwCurTime; // 현재 시각은 다음 호출시 예전 시각이 된다.

	// 키입력에 따른 위치 값 변경
	if (GetAsyncKeyState(VK_LEFT) < 0)
	{
		if (m_sPlayer.vPos.x - (m_dwElapsedTime * m_sPlayer.fVelocity) >= -6.0f) // 지면 밖을 나가지 못하게 설정
			m_sPlayer.vPos.x -= m_dwElapsedTime * m_sPlayer.fVelocity;
	}
	if (GetAsyncKeyState(VK_RIGHT) < 0)
	{
		if (m_sPlayer.vPos.x + (m_dwElapsedTime * m_sPlayer.fVelocity) <= 6.0f)
			m_sPlayer.vPos.x += m_dwElapsedTime * m_sPlayer.fVelocity;
	}

	if (GetAsyncKeyState(VK_UP) < 0)
	{
		if (m_sPlayer.vPos.z + (m_dwElapsedTime * m_sPlayer.fVelocity) <= 20.0f)
			m_sPlayer.vPos.z += m_dwElapsedTime * m_sPlayer.fVelocity;
	}
	if (GetAsyncKeyState(VK_DOWN) < 0)
	{
		if (m_sPlayer.vPos.z - (m_dwElapsedTime * m_sPlayer.fVelocity) >= -19.0f)
			m_sPlayer.vPos.z -= m_dwElapsedTime * m_sPlayer.fVelocity;
	}

	if (GetAsyncKeyState('S') < 0)  // 주인공 미사일 발사
	{
		if (dwCurTime - m_sPlayer.dwOldBulletFireTime >= m_sPlayer.dwBulletFireTime)
		{
			m_sPlayer.dwOldBulletFireTime = dwCurTime;

			for (i = 0; i < 10; i++)
			{
				if (m_sPlayerBullet[i].nLife <= 0)
				{
					m_sPlayerBullet[i].nLife = 1;
					m_sPlayerBullet[i].vPos = m_sPlayer.vPos;
					m_sPlayerBullet[i].vPos.z += 1.0f;
					break; // 총알을 한번의 한발씩 발사하려면 이렇게 break를 걸어주어야한다. 이게 없다면 10발이 겹쳐져 한발만 나가고 연발이 불가능 할 것이다.
				}
			}
		}
	}

	// Note: 주인공 총알 이동
	for (i = 0; i < 10; i++)
	{
		if (m_sPlayerBullet[i].nLife > 0)
		{
			m_sPlayerBullet[i].vPos.z += m_dwElapsedTime * m_sPlayerBulletProperty.fBulletVelcoty; // 시간 * 속력 = 거리, 아주 간단한 메커니즘
			if (m_sPlayerBullet[i].vPos.z >= 20.0f) // 경계 영역 충돌
			{
				m_sPlayerBullet[i].nLife = 0; // 충돌 했다면 체력을 0으로 하여 출력 대상에서 제외
			}
			else // 아니라면 이동한 만큼 이동 행렬을 생성
			{
				D3DXMatrixTranslation(&m_sPlayerBullet[i].matTranslation, m_sPlayerBullet[i].vPos.x, m_sPlayerBullet[i].vPos.y, m_sPlayerBullet[i].vPos.z);
			}
		}
	}

	D3DXMatrixTranslation(&m_sPlayer.matTranslation, m_sPlayer.vPos.x, m_sPlayer.vPos.y, m_sPlayer.vPos.z); // 이동 행렬 구하기
	m_sPlayer.matWorld = m_sPlayer.matScale * m_sPlayer.matRotationY * m_sPlayer.matTranslation; // SRT, 크기, 회전, 이동 행렬 최종 값

	dwAccumulateTime = dwAccumulateTime + m_dwElapsedTime; // 누적 시간은 지난 시간이 쌓여간다.
	if (dwAccumulateTime >= 1000) // 1초가 되었으면 FPS 출력
	{
		dwAccumulateTime = 0; // 누적 시간 초기화
		m_nFPS = m_nFPSCount; // 현재 까지 증가시킨 FPS 값을 화면에 출력하기 위해 m_nFPS에 넣어준다.
		m_nFPSCount = 0; // 다시 FPS 초기화
	}
}

void CGameEdu01::OnRelease()
{
	m_pFont->Release();
}