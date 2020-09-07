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

	D3DXCreateBox(m_pd3dDevices, 1.0f, 1.0f, 1.0f, &m_pMesh, NULL); // 박스 생성
	D3DXVECTOR3* pVertices; // 정점 버퍼들 저장할 포인터

	// 정점 버퍼 다른 곳에서 변경 못하게 LOCK, 밑 부분에서만 변경가능
	m_pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
	// 정점 버퍼가 담겨있는 포인터를 넣어주면 최대점과 최소점을 계산해서 도출해주는 함수
	D3DXComputeBoundingBox(pVertices, m_pMesh->GetNumVertices(), m_pMesh->GetNumBytesPerVertex(), &m_vMin, &m_vMax);
	m_pMesh->UnlockVertexBuffer(); // 다시 정점 버퍼 LOCK 해제

	D3DXMATRIX matScale, matTrans, matWorld; // 이동, 스케일 행렬

	// 박스_1 속성 초기화
	m_Box[0].fScaling = 2.0f;
	m_Box[0].vTans = D3DXVECTOR3(5.0f, 0.0f, 0.0f); // 처음 생성 좌표
	D3DXMatrixTranslation(&matTrans, m_Box[0].vTans.x, m_Box[0].vTans.y, m_Box[0].vTans.z); // 이동
	D3DXMatrixScaling(&matScale, m_Box[0].fScaling, m_Box[0].fScaling, m_Box[0].fScaling); // 스케일링
	matWorld = matScale * matTrans;
	// 위에서 최초로 구한 최대점(m_vMax), 최소점(m_vMin)을 이동, 스케일링을 거쳐 위치, 모양이 변한 큐브에 다시 알맞게 조정하는 함수
	D3DXVec3TransformCoord(&m_Box[0].vMin, &m_vMin, &matWorld); // m_vMin을 행렬변환을 통해 다시 큐브의 알맞은 최소점으로 변환
	D3DXVec3TransformCoord(&m_Box[0].vMax, &m_vMax, &matWorld); // m_vMax을 행렬변환을 통해 다시 큐브의 알맞은 최대점으로 변환

	// 박스_2 속성 초기화
	m_Box[1].fScaling = 1.0f;
	m_Box[1].vTans = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXMatrixTranslation(&matTrans, m_Box[1].vTans.x, m_Box[1].vTans.y, m_Box[1].vTans.z); // 이동
	D3DXMatrixScaling(&matScale, m_Box[1].fScaling, m_Box[1].fScaling, m_Box[1].fScaling); // 스케일링
	matWorld = matScale * matTrans;
	D3DXVec3TransformCoord(&m_Box[1].vMin, &m_vMin, &matWorld);
	D3DXVec3TransformCoord(&m_Box[1].vMax, &m_vMax, &matWorld);

	m_bIsCollision = FALSE; // 초반 충돌 X

	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
}

void CGameEdu01::OnRender()
{
	char string[100];
	RECT rt; // 글자를 위치시키기 위한 사각형
	D3DXMATRIX matWorld, matScale, matTrans;

	for (int i = 0; i < 2; i++) // 각 큐브 출력
	{
		D3DXMatrixTranslation(&matTrans, m_Box[i].vTans.x, m_Box[i].vTans.y, m_Box[i].vTans.z);
		D3DXMatrixScaling(&matScale, m_Box[i].fScaling, m_Box[i].fScaling, m_Box[i].fScaling);
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
		m_Box[0].vTans.x -= m_dwElapsedTime * 0.003f; // m_dwElapsedTime(시간), 0.003f(속도) -> 시간 x 속도 = 거리
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		m_Box[0].vTans.x += m_dwElapsedTime * 0.003f;
	}

	// 박스_2는 움직이지 않으니 박스_1만 최대점, 최소점 좌표를 이동한 만큼 변환해준다.
	D3DXMATRIX matScale, matTrans, matWorld;
	D3DXMatrixTranslation(&matTrans, m_Box[0].vTans.x, m_Box[0].vTans.y, m_Box[0].vTans.z);
	D3DXMatrixScaling(&matScale, m_Box[0].fScaling, m_Box[0].fScaling, m_Box[0].fScaling);
	matWorld = matScale * matTrans;
	D3DXVec3TransformCoord(&m_Box[0].vMin, &m_vMin, &matWorld);
	D3DXVec3TransformCoord(&m_Box[0].vMax, &m_vMax, &matWorld);

	m_bIsCollision = CheckCubeIntersection(&m_Box[0].vMin, &m_Box[0].vMax, &m_Box[1].vMin, &m_Box[1].vMax); // 충돌 체크
}

void CGameEdu01::OnRelease()
{
	m_pMesh->Release();
	m_pFont->Release();
}