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

	m_Eye.x = 0.0f;
	m_Eye.y = 0.1f;
	m_Eye.z = -10.0f;

	m_At.x = 0.0f; m_At.y = 0.0f; m_At.z = 0.0f;

	m_Up.x = 0.0f; m_Up.y = 1.0f; m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 뷰 변환 행렬 생성
	m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f); // 프로젝션 변환 행렬 생성 (투영)
	m_pd3dDevices->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevices->SetViewport(&vp); // 뷰 포트 설정

	// 삼각형 설정, 스트립 형태로 출력할거라 중복되는 정점이 없다.
	CUSTOMVERTEX vertices[] = {
		{ D3DXVECTOR3(-1.0f, 1.0f, 0.0f), 0.0f, 0.0f },
		{ D3DXVECTOR3(1.0f, 1.0f, 0.0f), 1.0f, 0.0f },
		{ D3DXVECTOR3(-1.0f, -1.0f, 0.0f), 0.0f, 1.0f },
		{ D3DXVECTOR3(1.0f, -1.0f, 0.0f), 1.0f, 1.0f },
	};

	// 일반적인 도형 정점 설정
	m_pd3dDevices->CreateVertexBuffer(sizeof(vertices), 0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &m_pVB, NULL);

	VOID* pVertices;
	m_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
	memcpy(pVertices, vertices, sizeof(vertices));
	m_pVB->Unlock();

	// 외부로부터 이미지를 읽어 텍스쳐 객체인 m_pTexture에 저장해준다.
	D3DXCreateTextureFromFile(m_pd3dDevices, "banana.bmp", &m_pTexture); // 이미지를 tu, tv에 맞게 설정해주는 편리한 함수

	// 스케일링 행렬 설정
	D3DXMatrixScaling(&m_matScale[0], 0.5f, 0.5f, 0.5f);
	D3DXMatrixScaling(&m_matScale[1], 1.5f, 1.5f, 1.5f);

	// 이동 행렬 설정
	D3DXMatrixTranslation(&m_matTranslate[0], -2.0f, 0.0f, 0.0f);
	D3DXMatrixTranslation(&m_matTranslate[1], 1.5f, 0.0f, 0.0f);

	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE);
}

void CGameEdu01::OnRender()
{
	D3DXMATRIX matWorld;

	m_pd3dDevices->SetTexture(0, m_pTexture); // 텍스쳐 설정, 앞의 숫자는 텍스쳐의 인덱스로 최대 8개까지의 텍스쳐를 섞어서 렌더링이 가능하다.
	m_pd3dDevices->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
	m_pd3dDevices->SetFVF(D3DFVF_CUSTOMVERTEX);

	for (int i = 0; i < 2; i++)
	{
		matWorld = m_matScale[i] * m_matTranslate[i];
		m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld); // ST 적용
		m_pd3dDevices->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2); // 삼각형 2개 연달아 스트립으로 출력, 스트립이라 정점 중복이 없다.
	}
}

void CGameEdu01::OnUpdate()
{
	
}

void CGameEdu01::OnRelease()
{
	if (m_pTexture != NULL)
	{
		m_pTexture->Release();
	}
	if (m_pVB != NULL)
	{
		m_pVB->Release();
	}
}