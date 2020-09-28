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

	m_Eye.x = 15.0f;
	m_Eye.y = 10.0f;
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
}

void CGameEdu01::OnRender()
{
	D3DXMATRIX matRotate, matTrans, matScaling, matWorld; // 위치 조정하기 위한 행렬
	float fScaling[3] = { 0.3f, 0.6f, 1.0f }; // 많이 작은거, 조금 작은거, 원래 크기
	D3DXVECTOR3 vTrans[3] = { D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(2.0f, 0.0f, 0.0f) , D3DXVECTOR3(5.0f, 0.0f, 0.0f) }; // 원 위치, x로 2이동, x로 5이동

	m_Axis.OnRender();
	//m_Cube.OnRender();

	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, FALSE); // 라이트 켜주면 안보이기에 일단 꺼줌
	m_pd3dDevices->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME); // 뼈대만 보이기

	D3DXMatrixRotationY(&matRotate, GetTickCount64() * 0.004f); // 회전 행렬 얻기

	// 첫번째 주전사 셋업 및 출력
	D3DXMatrixScaling(&matScaling, fScaling[0], fScaling[0], fScaling[0]);
	D3DXMatrixTranslation(&matTrans, vTrans[0].x, vTrans[0].y, vTrans[0].z);
	// SRT 순의 스케일링 -> 회전 -> 이동 연산
	matWorld = matScaling * matRotate * matTrans;
	m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld); // 월드, 즉 물체 좌표를 회전해서 위치시킨다.
	m_pTeapotMesh->DrawSubset(0); // 주전자 출력

	// 두번째 주전사 셋업 및 출력
	D3DXMatrixScaling(&matScaling, fScaling[1], fScaling[1], fScaling[1]);
	D3DXMatrixTranslation(&matTrans, vTrans[1].x, vTrans[1].y, vTrans[1].z);
	// SRT 순의 스케일링 -> 회전 -> 이동 연산
	matWorld = matScaling * matRotate * matTrans;
	m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld); // 월드, 즉 물체 좌표를 회전해서 위치시킨다.
	m_pTeapotMesh->DrawSubset(0); // 주전자 출력

	// 세번째 주전사 셋업 및 출력
	D3DXMatrixScaling(&matScaling, fScaling[2], fScaling[2], fScaling[2]);
	D3DXMatrixTranslation(&matTrans, vTrans[2].x, vTrans[2].y, vTrans[2].z);
	// STR 순의 스케일링 -> 이동 연산 -> 회전 (SRT가 정상이지 이건 잘 안쓴다.)
	// 이렇게 할 경우 회전의 축은 아직 기존의 축과 같아 기존 축 기준으로 크게 빙 도는 것 같이 표현된다.
	// 이동한 거리만큼이 회전원의 반지름이라고 생각하면 쉽다.
	matWorld = matScaling * matTrans * matRotate;
	m_pd3dDevices->SetTransform(D3DTS_WORLD, &matWorld); // 월드, 즉 물체 좌표를 회전해서 위치시킨다.
	m_pTeapotMesh->DrawSubset(0); // 주전자 출력

}

void CGameEdu01::OnUpdate()
{
	
}

void CGameEdu01::OnRelease()
{
	m_Axis.OnRelease();
	//m_Cube.OnRelease();

	m_pTeapotMesh->Release();
}