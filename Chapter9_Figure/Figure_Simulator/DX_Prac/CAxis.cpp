#include "CAxis.h"

CAxis::CAxis(void)
{
	m_pd3dDevice = NULL;
	m_pVB = NULL;
}


CAxis::~CAxis(void)
{
}


void CAxis::OnInit(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice;

	// 축 그리기, 축은 변하면 안되니 Init에서 처리한다.
	CUSTOMVERTEX vertices[] =
	{
		{ -3.0f,  0.0f, 0.0f, D3DCOLOR_RGBA(255, 0, 0, 255) },  // x 축 (빨간색)
		{ 3.0f,  0.0f, 0.0f, D3DCOLOR_RGBA(255, 0, 0, 255) },

		{ 0.0f,  -3.0f, 0.0f, D3DCOLOR_RGBA(0, 255, 0, 255) }, // y 축 (초록색)
		{ 0.0f,  3.0f, 0.0f, D3DCOLOR_RGBA(0, 255, 0, 255) },

		{ 0.0f,  0.0f, -3.0f, D3DCOLOR_RGBA(0, 255, 255, 255) }, // z 축 (마젠타색과 유사)
		{ 0.0f,  0.0f, 3.0f, D3DCOLOR_RGBA(0, 255, 255, 255) }		
	};

	// 정점 버퍼에 정점들 집어 넣기
	m_pd3dDevice->CreateVertexBuffer(6 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &m_pVB, NULL);

	VOID* pVertices;
	m_pVB->Lock(0, sizeof(vertices), &pVertices, 0);
	memcpy(pVertices, vertices, sizeof(vertices));
	m_pVB->Unlock();
}

void CAxis::OnRender()
{
	D3DXMATRIX matWorld;
	// 단위행렬(E), 선언한 정점(vertices)들에 어떠한 변화도 주지 않고 그대로 사용하기에 단위행렬 사용
	D3DXMatrixIdentity(&matWorld);

	// 빛과 같은 계산을 픽셀에다 해주는 설정
	// 면에 법선 벡터를 따지지 않고 단순하게 선을 긋는 것이기에 라이팅을 끈다.
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	
	// 정점 버퍼 사용
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
	m_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	
	// 모델 좌표계를 그대로 쓰겠다는 설정
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	// 선 그리기
	m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 3);

	// 다시 라이팅 킨 원래 상태로 복구
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
}

void CAxis::OnRelease()
{
	if (m_pVB != NULL)
		m_pVB->Release();
}