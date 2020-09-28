#include "CGameEdu01.h"

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

	m_Eye.x = 5.0f;
	m_Eye.y = 8.0f;
	m_Eye.z = -10.0f;

	m_At.x = 0.0f; m_At.y = 0.0f; m_At.z = 0.0f;

	m_Up.x = 0.0f; m_Up.y = 1.0f; m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 뷰 변환 행렬 생성
	m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f); // 프로젝션 변환 행렬 생성 (투영)
	m_pd3dDevices->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevices->SetViewport(&vp); // 뷰 포트 설정

	// 비스듬하게 삼각형 설정
	CUSTOMVERTEX vertices[3] = {
		{ D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_RGBA(100, 100, 100, 255) },
		{ D3DXVECTOR3(1.0f, -1.0f, 1.5f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_RGBA(100, 100, 100, 255) },
		{ D3DXVECTOR3(1.0f, -1.0f, -1.5f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_RGBA(100, 100, 100, 255) }
	};

	m_pd3dDevices->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &m_pVB, NULL);

	// 법선 벡터 계산
	D3DXVECTOR3 vS0, vS1;
	vS0 = vertices[2].vPos - vertices[0].vPos; // 벡터 V2 - V0 을 하여 벡터 ->V0 을 얻는다. 방향이 ->, 그냥 V0이 아니라!!!
	vS1 = vertices[1].vPos - vertices[0].vPos;
	D3DXVec3Cross(&vertices[0].vNormal, &vS1, &vS0); // 법선 벡터 구하기
	D3DXVec3Normalize(&vertices[0].vNormal, &vertices[0].vNormal); // 크기 1로 만들기

	// 하나의 삼각형을 구성하는 법선벡터는 모두 같다. 
	vertices[2].vNormal = vertices[1].vNormal = vertices[0].vNormal; // vNormal은 법선 벡터

	VOID* pVertices;
	m_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
	memcpy(pVertices, vertices, sizeof(vertices));
	m_pVB->Unlock();

	// 법선 벡터 출력을 위한 라인 == 7강의 축 클래스를 참조하면 쉽다.
	CROSSVERTEX vCrossVertices[6];
	vCrossVertices[0].vPos = vertices[0].vPos; // 얘는 그냥 삼각형 꼭짓점
	// 삼각형의 한 꼭짓점 + 법선 벡터 <-- 이것을 하는 이유는 법선을 그릴 때 필요한 점을 얻기 위해서다.
	vCrossVertices[1].vPos = vertices[0].vPos + vertices[0].vNormal;

	vCrossVertices[2].vPos = vertices[1].vPos;
	vCrossVertices[3].vPos = vertices[1].vPos + vertices[1].vNormal;

	vCrossVertices[4].vPos = vertices[2].vPos;
	vCrossVertices[5].vPos = vertices[2].vPos + vertices[2].vNormal;
	for (int i = 0; i < 6; i++)
	{
		vCrossVertices[i].dwDiffuse = D3DCOLOR_RGBA(255, 255, 255, 255);
	}

	m_pd3dDevices->CreateVertexBuffer(sizeof(vCrossVertices), 0, D3DFVF_CROSSVERTEX,
		D3DPOOL_DEFAULT, &m_pCrossVB, NULL);

	m_pCrossVB->Lock(0, sizeof(vertices), &pVertices, 0);
	memcpy(pVertices, vCrossVertices, sizeof(vCrossVertices));
	m_pCrossVB->Unlock();

	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE);
}

void CGameEdu01::OnRender()
{	
	m_pd3dDevices->SetFVF(D3DFVF_CUSTOMVERTEX);
	m_pd3dDevices->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
	m_pd3dDevices->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1); // 삼각형 그리기

	m_pd3dDevices->SetFVF(D3DFVF_CROSSVERTEX);
	m_pd3dDevices->SetStreamSource(0, m_pCrossVB, 0, sizeof(CROSSVERTEX));
	m_pd3dDevices->DrawPrimitive(D3DPT_LINELIST, 0, 3); // 법선 3개 그리기
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