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

	m_Axis.OnInit(m_pd3dDevices);
	//m_Cube.OnInit(m_pd3dDevices);

	// m_pTeapotMesh에 기존에 주전자 만들 때 했던 인덱스 버퍼, 정점 버퍼 등에 대한 정보다 다 들어있다.
	// 사용법은 함수에 주석으로 모두 쓰여있다.
	// 슬라이스는 피자, 케이크 자르듯이 자르는거, 스택은 소시지 자르듯이 자르는거 라고 생각하자
	// 슬라이스와 스택이 많아 질수록 더욱 부드러운 곡선을 만들 수 있지만 정점도 많아져 연산량도 증가한다.
	D3DXCreateTeapot( m_pd3dDevices, &m_pTeapotMesh, NULL ); // 주전자
	m_fScale = 1.0f;
}

void CGameEdu01::OnRender()
{
	D3DXMATRIX matScale; // 스케일 조정하기 위한 행렬

	m_Axis.OnRender();
	//m_Cube.OnRender();

	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE); // 라이트 켜주면 안보이기에 일단 꺼줌
	m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME); // 뼈대만 보이기
	m_pd3dDevices->SetRenderState( D3DRS_CULLMODE,  D3DCULL_NONE); // 컬링 제거, 뒷면 보기 가능

	D3DXMatrixScaling(&matScale, m_fScale, m_fScale, m_fScale); // 스케일링 해준다.
	m_pd3dDevices->SetTransform(D3DTS_WORLD, &matScale); // 월드, 즉 물체 좌표를 스케일링해서 위치시킨다.

	// 주전자 출력
    m_pTeapotMesh->DrawSubset(0);	
}

void CGameEdu01::OnUpdate()
{
	if (GetAsyncKeyState('A')<0)
	{
		m_fScale = m_fScale + 0.1f;
	}
	if (GetAsyncKeyState('D') < 0)
	{
		m_fScale = m_fScale - 0.1f;
	}
}

void CGameEdu01::OnRelease()
{
	m_Axis.OnRelease();
	//m_Cube.OnRelease();

	m_pTeapotMesh->Release();
}