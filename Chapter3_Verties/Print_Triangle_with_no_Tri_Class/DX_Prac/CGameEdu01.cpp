#include "CGameEdu01.h"

CGameEdu01::CGameEdu01()
{
	m_pVB = NULL;
}

CGameEdu01::~CGameEdu01()
{

}

void CGameEdu01::OnInit()
{
	CUSTOMVERTEX vertices[] =
	{
		{150.0f, 50.0f, 0.5f, 1.0f, 0xffff0000},
		{250.0f, 250.0f, 0.5f, 1.0f, 0xff00ff00},
		{50.0f, 250.0f, 0.5f, 1.0f, 0xff00ffff}
	};
	// 사용할 정점들을 정점 버퍼에 넣는다.
	m_pd3dDevices->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL);

	void* pVertices;
	m_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
	// 정점 버퍼에 있는 데이터들 만큼 메모리 정보를 포인터 pVertices에 옮겨 사용가능하게 만든다.
	memcpy(pVertices, vertices, sizeof(vertices)); 
	m_pVB->Unlock();
}

void CGameEdu01::OnRender()
{
	m_pd3dDevices->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX)); // 출력할 정점 버퍼를 디바이스의 데이터 스트림과 연결
	m_pd3dDevices->SetFVF(D3DFVF_CUSTOMVERTEX); // 정점 출력 포멧 설정
	m_pd3dDevices->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1); // 삼각형 1개 그리기
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
}