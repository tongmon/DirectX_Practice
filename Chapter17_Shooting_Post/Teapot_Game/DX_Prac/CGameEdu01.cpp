#include "CGameEdu01.h"

string g_strBGSound[] = { "Sound_Dir\\init.ogg", "Sound_Dir\\war.mp3", "Sound_Dir\\gameover.mp3" };
string g_strEffectSound[] = { "Sound_Dir\\story.mp3", "Sound_Dir\\stage.ogg", "Sound_Dir\\play_fire.wav", "Sound_Dir\\enemy_fire.wav", "Sound_Dir\\collision.wav" };

BOOL CheckCubeIntersection(D3DXVECTOR3* vMin1, D3DXVECTOR3* vMax1,
	D3DXVECTOR3* vMin2, D3DXVECTOR3* vMax2) // 큐브 충돌 체크 함수, 각 큐브의 최대점과 최소점을 비교하여 겹치는 부분이 있으면 충돌로 판단한다.
{
	if (vMin1->x <= vMax2->x && vMax1->x >= vMin2->x &&
		vMin1->y <= vMax2->y && vMax1->y >= vMin2->y &&
		vMin1->z <= vMax2->z && vMax1->z >= vMin2->z)
	{
		return TRUE;
	}
	return FALSE;
}

CGameEdu01::CGameEdu01()
{
	m_nGameState = INIT; // 게임 상태는 항상 처음엔 초기화부터 시작이다.
}

CGameEdu01::~CGameEdu01()
{

}

void CGameEdu01::OnInit()
{
	int i;
	RECT rect;
	D3DVIEWPORT9 vp;

	switch (m_nGameState) // 게임 진행 상태에 따라 실행되는 구간이 달라진다
	{
	case INIT: // 게임 처음 시작하면 실행되는 부분
		srand((unsigned int)time(NULL)); // 난수 생성을 위한 초기화
		GetClientRect(m_hWnd, &rect);

		// 뷰포트 설정
		vp.X = 0;
		vp.Y = 0;
		vp.Width = rect.right - rect.left;  // 좌표개념이 아니라 크기 개념
		vp.Height = rect.bottom - rect.top;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;

		// 카메라 위치 설정
		m_Eye.x = 0.0f;
		m_Eye.y = 0.0f;
		m_Eye.z = -12.0f; // 게임 시작 전 메뉴 화면이라 -12로 설정

		// 카메라가 바라보는 위치 설정
		m_At.x = 0.0f;
		m_At.y = 0.0f;
		m_At.z = 0.0f;

		m_Up.x = 0.0f;
		m_Up.y = 1.0f;
		m_Up.z = 0.0f;

		// 카메라 설정
		D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up);
		m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

		// 3D 투영 변환 설정
		D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
		m_pd3dDevices->SetTransform(D3DTS_PROJECTION, &m_matProj);
		m_pd3dDevices->SetViewport(&vp);

		// Note: 메쉬 생성
		D3DXCreateTeapot(m_pd3dDevices, &m_pTeapotMesh, NULL);
		D3DXCreateCylinder(m_pd3dDevices, 2.0f, 0.0f, 10.0f, 15, 10, &m_pPlayerBulletMesh, NULL);
		D3DXCreateBox(m_pd3dDevices, 1.0f, 1.0f, 1.0f, &m_pEnemyBoxMesh, NULL);
		D3DXCreateSphere(m_pd3dDevices, 0.1f, 20, 10, &m_pEnemyBulletMesh, NULL);
		m_Ground.Create(m_pd3dDevices, 20, 6, 2.0f);

		// 각 폰트 설정
		D3DXCreateFont(m_pd3dDevices, 50, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			"Arial", &m_pFont);
		D3DXCreateFont(m_pd3dDevices, 30, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			"Arial", &m_pFont2);
		D3DXCreateFont(m_pd3dDevices, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			"Arial", &m_pFont3);

		// 소리 설정
		m_FMODSound.CreateBGSound(3, g_strBGSound); // 배경음 3개
		m_FMODSound.PlaySoundBG(0); // 현재 배경음 첫번째 실행
		break;

	case LOADING: // 로딩 부분
		m_FMODSound.StopSoundBG(0); // 첫번째 배경음 출력 중지

		m_FMODSound.CreateEffectSound(5, g_strEffectSound); // 효과음 5개 설정
		// 주인공의 최대 최소 정점 값 <= 모델 좌표
		D3DXVECTOR3* pVertices;
		m_pTeapotMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
		D3DXComputeBoundingBox(pVertices, m_pTeapotMesh->GetNumVertices(), D3DXGetFVFVertexSize(m_pTeapotMesh->GetFVF()), &m_sPlayer.vMin, &m_sPlayer.vMax);
		m_pTeapotMesh->UnlockVertexBuffer();

		// 주인공 미사일 최대 최소정점 구하기
		m_pPlayerBulletMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
		D3DXComputeBoundingBox(pVertices, m_pPlayerBulletMesh->GetNumVertices(), D3DXGetFVFVertexSize(m_pPlayerBulletMesh->GetFVF()), &m_sPlayerBulletProperty.vMin, &m_sPlayerBulletProperty.vMax);
		m_pPlayerBulletMesh->UnlockVertexBuffer();

		// 적 충돌 좌표 	
		m_pEnemyBoxMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
		D3DXComputeBoundingBox(pVertices, m_pEnemyBoxMesh->GetNumVertices(), D3DXGetFVFVertexSize(m_pEnemyBoxMesh->GetFVF()), &m_EnemyProperty.vMin, &m_EnemyProperty.vMax);
		m_pEnemyBoxMesh->UnlockVertexBuffer();

		// 적 총알 
		m_pEnemyBulletMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
		D3DXComputeBoundingBox(pVertices, m_pEnemyBoxMesh->GetNumVertices(), D3DXGetFVFVertexSize(m_pEnemyBulletMesh->GetFVF()), &m_EnemyBulletProperty.vMin, &m_EnemyBulletProperty.vMax);
		m_pEnemyBulletMesh->UnlockVertexBuffer();
		break;

	case READY: // 준비 단계
		m_FMODSound.PlaySoundEffect(1); // 효과음 두번째 출력

		// 게임 뷰로 설정
		m_Eye.x = 0.0f;
		m_Eye.y = 10.0f;
		m_Eye.z = -32.0f; // 게임 시작이 되는 뷰로 -32로 설정

		m_At.x = 0.0f;
		m_At.y = 0.0f;
		m_At.z = 0.0f;

		m_Up.x = 0.0f;
		m_Up.y = 1.0f;
		m_Up.z = 0.0f;

		// 카메라 다시 설정
		D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up);
		m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

		// Note: 주인공 설정	
		m_sPlayer.nLife = 5; // 플레이어 생명
		m_sPlayer.fScale = 0.7f;
		m_sPlayer.fRotationY = -D3DXToRadian(90);
		m_sPlayer.vPos = D3DXVECTOR3(0.0f, 0.0f, -9 * 2.0f);
		m_sPlayer.fVelocity = 0.005f;
		m_sPlayer.dwBulletFireTime = 400;
		m_sPlayer.dwOldBulletFireTime = GetTickCount64();
		for (i = 0; i < 10; i++)
		{
			m_sPlayerBullet[i].nLife = 0;
		}

		// 크기, 이동, 회전 변환 행렬 생성
		D3DXMatrixScaling(&m_sPlayer.matScale, m_sPlayer.fScale, m_sPlayer.fScale, m_sPlayer.fScale);
		D3DXMatrixTranslation(&m_sPlayer.matTranslation, m_sPlayer.vPos.x, m_sPlayer.vPos.y, m_sPlayer.vPos.z);
		D3DXMatrixRotationY(&m_sPlayer.matRotationY, m_sPlayer.fRotationY);

		// Note: 주인공 총알 설정
		m_sPlayerBulletProperty.fBulletVelcoty = 0.01f;
		m_sPlayerBulletProperty.fScale = 0.08f;
		D3DXMatrixScaling(&m_sPlayerBulletProperty.matScale, m_sPlayerBulletProperty.fScale, m_sPlayerBulletProperty.fScale, m_sPlayerBulletProperty.fScale);

		// Note: 적 캐릭터 설정
		m_EnemyProperty.fScale = 1.0f;
		m_EnemyProperty.fMoveVelcoty = 0.003f;
		m_EnemyProperty.dwFireTime = 1600;
		D3DXMatrixScaling(&m_EnemyProperty.matScale, m_EnemyProperty.fScale, m_EnemyProperty.fScale, m_EnemyProperty.fScale);

		// 적 등장 설정
		int nRandomNum;
		for (i = 0; i < EnemyCount; i++)
		{
			m_Enemy[i].nLife = 0;
			m_Enemy[i].dwAppearTime = 2000 + (i * 800);
			m_Enemy[i].dwOldFireTime = GetTickCount64();
			nRandomNum = rand() % 7;
			if (nRandomNum % 2)
				m_Enemy[i].vPos.x = -1.0f * nRandomNum;
			else
				m_Enemy[i].vPos.x = (float)nRandomNum;

			m_Enemy[i].vPos.y = 0.0f;
			m_Enemy[i].vPos.z = 20.0f;

			D3DXMatrixTranslation(&m_Enemy[i].matTranslation, m_Enemy[i].vPos.x, m_Enemy[i].vPos.y, m_Enemy[i].vPos.z);
		}

		// Note: 적 총알 설정
		m_EnemyBulletProperty.fBulletVelcoty = 0.01f;
		m_EnemyBulletProperty.fScale = 0.9f;
		D3DXMatrixScaling(&m_EnemyBulletProperty.matScale, m_EnemyBulletProperty.fScale, m_EnemyBulletProperty.fScale, m_EnemyBulletProperty.fScale);
		for (i = 0; i < 100; i++)
		{
			m_EnemyBullet[i].nLife = 0;
		}

		m_nEnemyIndex = 0; // 적 인덱스 초기화
		m_nGrade = 0; // 점수 초기화
		m_dwGameStartTime = GetTickCount64(); // 초기화가 끝났으니 게임 시작 시간 기록
		break;
	}
}

// 값에 따라 화면에 보여주기만 하는 부분, 출력만 담당하고 실제 값에 대한 컨트롤은 Update에서 처리하게 된다.
// 따라서 다음 단계로 넘기는 것은 렌더 부분에서 해주지 않는다.
void CGameEdu01::OnRender() 
{
	int i;
	D3DXMATRIX matWorld, matScale, matTrans, matRotation;
	RECT rt;
	char string[100];

	switch (m_nGameState) // 렌더링도 똑같이 상태에 따라 나뉘어 처리하게 된다.
	{
	case INIT: // 게임 시작 부분, 메뉴에 해당한다.
		// 주전자 Y축 회전하면서 출력
		D3DXMatrixScaling(&matScale, 2.0f, 2.0f, 2.0f);
		D3DXMatrixTranslation(&matTrans, 0.0f, -1.0f, 0.0f);
		D3DXMatrixRotationY(&matRotation, GetTickCount64() * 0.001f); // 시간에 따라 메쉬 회전 시킴
		m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		matWorld = matScale * matRotation * matTrans; // SRT 변환
		m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld);
		m_pTeapotMesh->DrawSubset(0);

		// 총알 X축 회전하면서 출력
		D3DXMatrixScaling(&matScale, 0.1f, 0.1f, 0.1f);
		D3DXMatrixTranslation(&matTrans, 0.0f, 1.5f, 0.0f);
		D3DXMatrixRotationX(&matRotation, GetTickCount64() * 0.002f);
		matWorld = matScale * matRotation * matTrans;
		m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld);
		m_pPlayerBulletMesh->DrawSubset(0);

		// 텍스트 게임 제목 출력
		SetRect(&rt, 360, 130, 0, 0);
		m_pFont->DrawText(NULL, "3D 주전자 게임", -1, &rt, DT_NOCLIP,
			D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		SetRect(&rt, 360, 800, 0, 0);
		m_pFont2->DrawText(NULL, "Space 키를 눌러 게임 시작!", -1, &rt, DT_NOCLIP,
			D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		break;

	case LOADING: // 로딩 부분
		SetRect(&rt, 150, 430, 0, 0);
		m_pFont->DrawText(NULL, "Loading ...", -1, &rt, DT_NOCLIP,
			D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		// 게이지 채우기
		for (i = 0; i < m_nLoadingCount; i++)
		{
			SetRect(&rt, 200 + i * 10, 500, 0, 0);
			m_pFont2->DrawText(NULL, "■", -1, &rt, DT_NOCLIP,
				D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		}
		break;

	case READY: // 준비 단계
		SetRect(&rt, 400, 250, 0, 0);
		wsprintf(string, "Stage %d", m_nStage);
		m_pFont->DrawText(NULL, string, -1, &rt, DT_NOCLIP,
			D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		m_nEnemyCount = EnemyCount;
		break;

	case RUN: // 실제 실행 상태
		m_Ground.OnRender(); // 지면 출력

		m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

		// 주인공 총알
		for (i = 0; i < 10; i++)
		{
			if (m_sPlayerBullet[i].nLife > 0)
			{
				matWorld = m_sPlayerBulletProperty.matScale * m_sPlayerBullet[i].matTranslation;
				m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld);
				m_pPlayerBulletMesh->DrawSubset(0);
			}
		}

		// 적 총알 
		for (i = 0; i < 100; i++)
		{
			if (m_EnemyBullet[i].nLife > 0)
			{
				matWorld = m_EnemyBulletProperty.matScale * m_EnemyBullet[i].matTranslation;
				m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld);
				m_pEnemyBulletMesh->DrawSubset(0);
			}
		}

		// 적 캐릭터		
		for (i = 0; i < m_nEnemyIndex; i++)
		{
			if (m_Enemy[i].nLife)
			{
				matWorld = m_EnemyProperty.matScale * m_Enemy[i].matTranslation;
				m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld);
				m_pEnemyBoxMesh->DrawSubset(0);
			}
		}

		// 주인공 	
		m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME); // 와이어 프레임으로 출력
		m_pd3dDevices->SetTransform(D3DTS_WORLD, &m_sPlayer.matWorld);
		m_pTeapotMesh->DrawSubset(0);
		m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, TRUE);

		SetRect(&rt, 10, 20, 0, 0);
		wsprintf(string, "Total Score : %d   Score :  %d", m_nTotalGrade, m_nGrade);
		m_pFont3->DrawText(NULL, string, -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));

		SetRect(&rt, 680, 20, 0, 0);
		wsprintf(string, "%s", "♥"); // 플레이어 체력 출력
		for (i = 0; i < m_sPlayer.nLife; i++)
		{
			rt.left = 680 + i * 15; // 한글자 크기가 15여서 15씩 밀면서 출력
			m_pFont2->DrawText(NULL, string, -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		}
		break;

	case SUCCESS: // 성공
		SetRect(&rt, 400, 130, 0, 0);
		m_pFont->DrawText(NULL, "SUCCESS", -1, &rt, DT_NOCLIP,
			D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		break;

	case FAILED: // 실패
		SetRect(&rt, 360, 230, 0, 0);
		m_pFont2->DrawText(NULL, "다시 하시겠습니까? (Yes/No)", -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		break;

	case END: // 게임 오버
		SetRect(&rt, 380, 280, 0, 0);
		wsprintf(string, "Total Score : %d", m_nTotalGrade);
		m_pFont2->DrawText(NULL, string, -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		SetRect(&rt, 380, 320, 0, 0);
		wsprintf(string, "Q를 눌러 게임 종료");
		m_pFont2->DrawText(NULL, string, -1, &rt, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		break;
	}
}

void CGameEdu01::OnUpdate()
{
	int i, j;
	static DWORD OldTime;
	DWORD dwCurTime = GetTickCount64();
	m_dwElapsedTime = dwCurTime - OldTime; // 경과 시간
	OldTime = dwCurTime;
	D3DXVECTOR3 vMin1, vMax1, vMin2, vMax2; // 충돌점 저장 버퍼
	D3DXMATRIX matWorld;
	float fTemp;
	static DWORD dwReadyTime; // 준비 경과 시간을 누적시키기 위해서

	// 게임 경과 시간 체크 부분
	m_dwGameElapsedTime = dwCurTime - m_dwGameStartTime;

	switch (m_nGameState)
	{
	case INIT: // SPACE 키를 입력 받아 INIT 상태를 LOADING 으로 바꾸어 준다.
		if (GetAsyncKeyState(VK_SPACE) < 0)
		{
			m_nGameState = LOADING;
			OnInit(); // 로딩 상태의 초기화 진행
		}
		break;

	case LOADING: 
		Sleep(100);
		m_nLoadingCount++; // 로딩 카운트 증가
		if (m_nLoadingCount == 50) // 로딩 카운터가 50이 되면 READY 상태로 바꾼다.
		{
			m_nGameState = READY;
			m_nStage++;
			dwReadyTime = 0;
			OnInit(); // READY 상태의 초기화 진행
		}
		break;

	case READY:
		dwReadyTime += m_dwElapsedTime; // 경과 시간을 계속 더함, 2초동안 READY 문구를 보여주기 위함
		if (dwReadyTime >= 2000) //3초면 게임으로 진행
		{
			m_nGameState = RUN; // 게임 상태를 RUN으로 변경
			m_FMODSound.PlaySoundBG(1); // 본 게임 배경음
			dwReadyTime = 0;
		}
		break;

	case RUN: // 게임 진행 부분, 실제 조작이 여기서 진행된다.
		if (GetAsyncKeyState(VK_LEFT) < 0)
		{
			if (m_sPlayer.vPos.x - (m_dwElapsedTime * m_sPlayer.fVelocity) >= -6.0f)
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

						m_FMODSound.PlaySoundEffect(2); // 발사 효과음 출력
						break;
					}
				}
			}
		}

		// 적 캐릭터 출현
		for (i = m_nEnemyIndex; i < EnemyCount; i++)
		{
			if (m_Enemy[i].dwAppearTime <= m_dwGameElapsedTime)
			{
				m_Enemy[i].nLife = 1;
				m_nEnemyIndex++;
			}
			else 
			{
				break;
			}
		}

		// 적 캐릭터 이동과 적 캐릭터 총알 발사
		for (i = 0; i < m_nEnemyIndex; i++)
		{
			if (m_Enemy[i].nLife)
			{
				m_Enemy[i].vPos.z -= m_dwElapsedTime * m_EnemyProperty.fMoveVelcoty;
				if (m_Enemy[i].vPos.z <= -20.0f) // 경계 영역
				{
					m_Enemy[i].nLife = 0;
					m_nEnemyCount--;    // 적 캐릭터의 개수 0 이되면 종료 
					if (m_nEnemyCount == 0)
					{
						m_nGameState = STOP;
					}
					continue;
				}

				D3DXMatrixTranslation(&m_Enemy[i].matTranslation, m_Enemy[i].vPos.x, m_Enemy[i].vPos.y, m_Enemy[i].vPos.z);

				// Note: 살아 있는 적 캐릭터만이 총알을 발사할 수 있으므로
				if (dwCurTime - m_Enemy[i].dwOldFireTime >= m_EnemyProperty.dwFireTime)
				{
					m_Enemy[i].dwOldFireTime = dwCurTime;
					for (j = 0; j < 100; j++)
					{
						if (!m_EnemyBullet[j].nLife)
						{
							m_EnemyBullet[j].nLife = 1;
							m_EnemyBullet[j].vPos = m_Enemy[i].vPos;
							m_EnemyBullet[j].vPos.z -= 0.5f; // 약간 앞에서 총알 발사가 되도록 하기 위해									
							break; // <-- 이게 중요하다. 총알 하나만 살리는 부분의 끝 
						}
					}
				}
			}
		}

		// 적 캐릭터 총알 이동
		for (i = 0; i < 100; i++)
		{
			if (m_EnemyBullet[i].nLife > 0)
			{
				m_EnemyBullet[i].vPos.z -= m_dwElapsedTime * m_EnemyBulletProperty.fBulletVelcoty;
				if (m_EnemyBullet[i].vPos.z <= -20.0f) // 경계 영역 충돌
					m_EnemyBullet[i].nLife = 0;
				else
					D3DXMatrixTranslation(&m_EnemyBullet[i].matTranslation, m_EnemyBullet[i].vPos.x, m_EnemyBullet[i].vPos.y, m_EnemyBullet[i].vPos.z);
			}
		}

		// Note: 주인공 총알 이동
		for (i = 0; i < 10; i++)
		{
			if (m_sPlayerBullet[i].nLife > 0)
			{
				m_sPlayerBullet[i].vPos.z += m_dwElapsedTime * m_sPlayerBulletProperty.fBulletVelcoty;
				if (m_sPlayerBullet[i].vPos.z >= 20.0f) // 경계 영역 충돌
					m_sPlayerBullet[i].nLife = 0;
				else
					D3DXMatrixTranslation(&m_sPlayerBullet[i].matTranslation, m_sPlayerBullet[i].vPos.x, m_sPlayerBullet[i].vPos.y, m_sPlayerBullet[i].vPos.z);
			}
		}

		// Note: 주인공과 적 캐릭터 충돌
		D3DXMatrixTranslation(&m_sPlayer.matTranslation, m_sPlayer.vPos.x, m_sPlayer.vPos.y, m_sPlayer.vPos.z);

		// 주인공의 충돌 박스
		matWorld = m_sPlayer.matScale * m_sPlayer.matRotationY * m_sPlayer.matTranslation; // SRT 변환
		D3DXVec3TransformCoord(&vMin1, &m_sPlayer.vMin, &matWorld);
		D3DXVec3TransformCoord(&vMax1, &m_sPlayer.vMax, &matWorld);

		if (vMin1.x > vMax1.x)  // 회전이 있으므로 최소의 값을 재계산한다.
		{
			fTemp = vMin1.x;
			vMin1.x = vMax1.x;
			vMax1.x = fTemp;
		}
		if (vMin1.y > vMax1.y)
		{
			fTemp = vMin1.y;
			vMin1.y = vMax1.y;
			vMax1.y = fTemp;
		}
		if (vMin1.z > vMax1.z)
		{
			fTemp = vMin1.z;
			vMin1.z = vMax1.z;
			vMax1.z = fTemp;
		}

		// 주인공 몸통과 적 몸통 충돌
		for (i = 0; i < m_nEnemyIndex; i++)
		{
			if (m_Enemy[i].nLife > 0)
			{
				D3DXMatrixTranslation(&m_Enemy[i].matTranslation, m_Enemy[i].vPos.x, m_Enemy[i].vPos.y, m_Enemy[i].vPos.z);
				matWorld = m_EnemyProperty.matScale * m_Enemy[i].matTranslation;

				D3DXVec3TransformCoord(&vMin2, &m_EnemyProperty.vMin, &matWorld);
				D3DXVec3TransformCoord(&vMax2, &m_EnemyProperty.vMax, &matWorld);

				if (CheckCubeIntersection(&vMin1, &vMax1, &vMin2, &vMax2))
				{
					m_Enemy[i].nLife = 0;
					m_sPlayer.nLife--;
					m_nEnemyCount--;    // 적 캐릭터의 개수 0 이되면 종료 
					m_nGrade += 10; // 주인공이 충돌하면서 죽은 것이므로 점수 가산
					m_FMODSound.PlaySoundEffect(4);

					if (m_sPlayer.nLife == 0)
					{
						m_nGameState = STOP;
					}

					if (m_nEnemyCount == 0)
					{
						m_nGameState = STOP;
					}
				}
			}
		}

		// Note: 주인공과 적 총알 충돌
		for (i = 0; i < 100; i++)
		{
			if (m_EnemyBullet[i].nLife > 0)
			{
				D3DXMatrixTranslation(&m_EnemyBullet[i].matTranslation, m_EnemyBullet[i].vPos.x, m_EnemyBullet[i].vPos.y, m_EnemyBullet[i].vPos.z);
				matWorld = m_EnemyBullet[i].matTranslation;
				D3DXVec3TransformCoord(&vMin2, &m_EnemyBulletProperty.vMin, &matWorld);
				D3DXVec3TransformCoord(&vMax2, &m_EnemyBulletProperty.vMax, &matWorld);

				if (CheckCubeIntersection(&vMin1, &vMax1, &vMin2, &vMax2))
				{
					m_EnemyBullet[i].nLife = 0;
					m_sPlayer.nLife--;
					m_FMODSound.PlaySoundEffect(4);
					if (m_sPlayer.nLife == 0)
					{
						m_nGameState = STOP;
					}
				}
			}
		}

		// Note: 주인공 총알과 적 캐릭터 충돌
		for (i = 0; i < 10; i++)
		{
			if (m_sPlayerBullet[i].nLife > 0)
			{
				D3DXMatrixTranslation(&m_sPlayerBullet[i].matTranslation, m_sPlayerBullet[i].vPos.x, m_sPlayerBullet[i].vPos.y, m_sPlayerBullet[i].vPos.z);
				matWorld = m_sPlayerBulletProperty.matScale * m_sPlayerBullet[i].matTranslation;
				D3DXVec3TransformCoord(&vMin1, &m_sPlayerBulletProperty.vMin, &matWorld);
				D3DXVec3TransformCoord(&vMax1, &m_sPlayerBulletProperty.vMax, &matWorld);

				// Note: 적 캐릭터와 충돌 체크
				for (j = 0; j < m_nEnemyIndex; j++)
				{
					if (m_Enemy[j].nLife > 0)
					{
						D3DXMatrixTranslation(&m_Enemy[j].matTranslation, m_Enemy[j].vPos.x, m_Enemy[j].vPos.y, m_Enemy[j].vPos.z);
						matWorld = m_EnemyProperty.matScale * m_Enemy[j].matTranslation;

						D3DXVec3TransformCoord(&vMin2, &m_EnemyProperty.vMin, &matWorld);
						D3DXVec3TransformCoord(&vMax2, &m_EnemyProperty.vMax, &matWorld);

						if (CheckCubeIntersection(&vMin1, &vMax1, &vMin2, &vMax2))
						{
							m_Enemy[j].nLife = 0;
							m_sPlayerBullet[i].nLife = 0;
							m_nGrade += 10; // 점수 가산

							m_nEnemyCount--;    // 적 캐릭터의 개수 0 이되면 종료 
							m_FMODSound.PlaySoundEffect(4);
							if (m_nEnemyCount == 0)
							{
								m_nGameState = STOP;
							}
							break;
						}
					}
				}

				// Note: 주인공 총알과 적 총알 충돌 체크
				if (m_sPlayerBullet[i].nLife > 0)
				{
					for (j = 0; j < 100; j++)
					{
						if (m_EnemyBullet[j].nLife > 0)
						{
							D3DXMatrixTranslation(&m_EnemyBullet[j].matTranslation, m_EnemyBullet[j].vPos.x, m_EnemyBullet[j].vPos.y, m_EnemyBullet[j].vPos.z);
							matWorld = m_EnemyBullet[j].matTranslation;
							D3DXVec3TransformCoord(&vMin2, &m_EnemyBulletProperty.vMin, &matWorld);
							D3DXVec3TransformCoord(&vMax2, &m_EnemyBulletProperty.vMax, &matWorld);

							if (CheckCubeIntersection(&vMin1, &vMax1, &vMin2, &vMax2))
							{
								m_EnemyBullet[j].nLife = 0;
								m_sPlayerBullet[i].nLife = 0;
								m_FMODSound.PlaySoundEffect(4);
								break;
							}
						}
					}
				}
			}
		}

		D3DXMatrixTranslation(&m_sPlayer.matTranslation, m_sPlayer.vPos.x, m_sPlayer.vPos.y, m_sPlayer.vPos.z);
		m_sPlayer.matWorld = m_sPlayer.matScale * m_sPlayer.matRotationY * m_sPlayer.matTranslation;
		break;

	case STOP:
		m_FMODSound.StopSoundBG(1); // 배경음 출력 중지
		if (m_sPlayer.nLife <= 0 || m_nEnemyCount > 0) // 플레이어 체력이 0 이하인데 적 수가 남아있으면 실패
		{
			m_nGameState = FAILED;
			m_FMODSound.PlaySoundBG(2); // 실패 효과음
		}
		else 
		{
			m_nGameState = SUCCESS;
		}

		dwReadyTime = 0; // 어떤 단계의 2초 정도 머무르게 해주는 변수 다시 초기화
		break;

	case SUCCESS:
		dwReadyTime += m_dwElapsedTime;
		if (dwReadyTime >= 3000) //3초면 게임으로 진행
		{
			m_nGameState = READY; // 다시 준비 상태로 되돌림
			m_nStage++;
			m_nTotalGrade += m_nGrade;
			dwReadyTime = 0;
			OnInit();
		}
		break;

	case FAILED:
		if (GetAsyncKeyState('Y') < 0)
		{
			m_nGameState = READY;
			m_nGrade = 0;
			m_FMODSound.StopSoundBG(2);
			dwReadyTime = 0;
			OnInit();
			break;
		}
		if (GetAsyncKeyState('N') < 0)
		{
			m_nGameState = END; // 게임 종료
			dwReadyTime = 0;
			m_nTotalGrade += m_nGrade;
			m_FMODSound.StopSoundBG(2);
			break;
		}
		break;

	case END:
		if (GetAsyncKeyState('Q') < 0)
		{
			Cleanup();
			PostQuitMessage(0);
		}
		break;
	}

	m_FMODSound.Update();
}

void CGameEdu01::OnRelease()
{
	m_FMODSound.ReleaseSound();
	m_pFont->Release();
	m_pFont2->Release();
	m_pFont3->Release();
	m_pEnemyBoxMesh->Release();
	m_pEnemyBulletMesh->Release();
	m_pTeapotMesh->Release();
	m_pPlayerBulletMesh->Release();
	m_Ground.OnRelease();
}