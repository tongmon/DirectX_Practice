#include "CGameEdu01.h"

CGameEdu01::CGameEdu01()
{
	
}

CGameEdu01::~CGameEdu01()
{

}

void CGameEdu01::SetMaterial(float r, float g, float b) // 물체의 디퓨즈(색상)값 설정 함수
{
	D3DCOLORVALUE Diffuse;
	Diffuse.r = r;
	Diffuse.g = g;
	Diffuse.b = b;
	Diffuse.a = 1.0f; // 투명도
	ZeroMemory(&m_Material, sizeof(D3DMATERIAL9));
	m_Material.Ambient = m_Material.Diffuse = Diffuse; // 어짜피 방향광이기에 Ambient 안써서 모두 1.0f로 설정
	m_pd3dDevices->SetMaterial(&m_Material); // 물질 설정
}

void CGameEdu01::SetDirectionalLight() // 광원 설정
{
	m_pd3dDevices->SetLight(0, &m_Light); // 빛을 하나만 설정하기에 SetLight 함수가 하나고 ID는 0이다.
	m_pd3dDevices->LightEnable(0, TRUE);
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
	m_Eye.y = 4.0f;
	m_Eye.z = -4.0f;

	m_At.x = 0.0f; m_At.y = 0.0f; m_At.z = 0.0f;

	m_Up.x = 0.0f; m_Up.y = 1.0f; m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 뷰 변환 행렬 생성
	m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f); // 프로젝션 변환 행렬 생성 (투영)
	m_pd3dDevices->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevices->SetViewport(&vp); // 뷰 포트 설정

	// 삼각형 지면 설정
	CUSTOMVERTEX vertices[] = {
		{ D3DXVECTOR3(-1.0f, -0.3f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(-1.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },

		{ D3DXVECTOR3(-1.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },

		{ D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, -0.3f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },

		{ D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, -0.3f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },

		{ D3DXVECTOR3(-1.0f, 0.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(-1.0f, -0.3f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(0.0f, 0.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },

		{ D3DXVECTOR3(-1.0f, -0.3f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(0.0f, 0.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },

		{ D3DXVECTOR3(0.0f, 0.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, 0.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },

		{ D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, -0.3f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, 0.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f) }
	};

	m_pd3dDevices->CreateVertexBuffer(sizeof(vertices), 0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &m_pVB, NULL);

	// 법선 벡터 계산
	D3DXVECTOR3 vS0, vS1;
	for (int i = 0; i < 8; i++) // 삼각형 폴리곤이 8개이고 각 8개 마다 법선 벡터를 설정해준다.
	{
		// 시작점 기준
		vS0 = vertices[i * 3 + 2].vPos - vertices[i * 3].vPos; // 변 벡터 1
		vS1 = vertices[i * 3 + 1].vPos - vertices[i * 3].vPos; // 변 벡터 2
		D3DXVec3Cross(&vertices[i * 3].vNormal, &vS1, &vS0);
		D3DXVec3Normalize(&vertices[i * 3].vNormal, &vertices[i * 3].vNormal);

		// 하나의 삼각형을 구성하는 법선벡터는 모두 같다. 
		vertices[i * 3 + 2].vNormal = vertices[i * 3 + 1].vNormal = vertices[i * 3].vNormal;
	}

	VOID* pVertices;
	m_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
	memcpy(pVertices, vertices, sizeof(vertices));
	m_pVB->Unlock();

	// 법선 벡터 출력을 위한 라인 == 7강의 축 클래스를 참조하면 쉽다.
	CROSSVERTEX vCrossVertices[48]; // 각 정점마다 법'선' 설정을 해주기 위해 6(선을 그리려면 점 2개고 삼각형 마다 3개 선이니 6개) * 8(삼각형 개수)

	for (int i = 0; i < 24; i++) // 한번에 vCrossVertices를 두개 씩 처리하기 때문에 24번만 진행한다.
	{
		vCrossVertices[i * 2].vPos = vertices[i].vPos;
		vCrossVertices[(i * 2) + 1].vPos = vertices[i].vPos + vertices[i].vNormal; // 법선 벡터 만큼 떨어진 정점 좌표 구하기

		vCrossVertices[(i * 2) + 1].dwDiffuse = vCrossVertices[i * 2].dwDiffuse = D3DCOLOR_RGBA(255, 255, 255, 255); // 무색 설정
	}

	m_pd3dDevices->CreateVertexBuffer(sizeof(vCrossVertices), 0, D3DFVF_CROSSVERTEX,
		D3DPOOL_DEFAULT, &m_pCrossVB, NULL);

	m_pCrossVB->Lock(0, sizeof(vertices), &pVertices, 0);
	memcpy(pVertices, vCrossVertices, sizeof(vCrossVertices));
	m_pCrossVB->Unlock();

	// 기본적인 머티리얼 초기 설정
	SetMaterial(1.0f, 1.0f, 1.0f);

	// 라이트 기본 초기화
	m_Light.Type = D3DLIGHT_DIRECTIONAL; // 게임에서 많이 쓰는 방향성 라이트
	m_Light.Diffuse.r = m_Light.Diffuse.g = m_Light.Diffuse.b = m_Light.Diffuse.a = 1.0f;

	// 라이트 속성 설정
	m_Light.Specular = m_Light.Diffuse;
	m_Light.Ambient.g = m_Light.Ambient.b = m_Light.Ambient.r = 0.7f;
	m_Light.Ambient.a = 1.0f;
	
	// 라이트 방향
	m_Light.Direction.x = -1.0f;
	m_Light.Direction.y = -0.4f;
	m_Light.Direction.z = 0.5f;

	SetDirectionalLight(); // 라이트 ID 설정, 라이트 켜주기
}

void CGameEdu01::OnRender()
{	
	m_pd3dDevices->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
	m_pd3dDevices->SetFVF(D3DFVF_CUSTOMVERTEX);
	m_pd3dDevices->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 8);

	/*
	// 법선 출력
	m_pd3dDevice->SetFVF( D3DFVF_CROSSVERTEX );
	m_pd3dDevice->SetStreamSource( 0, m_pCrossVB, 0, sizeof(CROSSVERTEX) );
	m_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, 24 );	
	*/
}

void CGameEdu01::OnUpdate()
{

}

void CGameEdu01::OnRelease()
{
	if (m_pVB != NULL)
	{
		m_pVB->Release();
	}
	if (m_pCrossVB != NULL)
	{
		m_pCrossVB->Release();
	}
}