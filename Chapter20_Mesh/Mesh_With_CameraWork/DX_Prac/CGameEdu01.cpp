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
	m_Eye.y = 3.0f;
	m_Eye.z = -10.0f;

	m_At.x = 0.0f; m_At.y = 0.0f; m_At.z = 0.0f;

	m_Up.x = 0.0f; m_Up.y = 1.0f; m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 뷰 변환 행렬 생성
	m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f); // 프로젝션 변환 행렬 생성 (투영)
	m_pd3dDevices->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevices->SetViewport(&vp); // 뷰 포트 설정

	// DX에서는 보통 여러 개의 정보가 배열로 담기는 경우 LPD3DXBUFFER 이러한 버퍼 포인터 형태를 사용한다.
	LPD3DXBUFFER pD3DXMtrlBuffer; // 이러한 버퍼 형태에 담기고 이를 알맞게 사용한다. 

	D3DXLoadMeshFromX("Tiger.x", D3DXMESH_SYSTEMMEM, m_pd3dDevices, NULL,
		&pD3DXMtrlBuffer, NULL, &m_dwNumMaterials, &m_pMesh); // .x 파일로부터 필요한 정보(텍스쳐[tu, tv], 정점 메쉬, 머터리얼)를 담아오는 파싱 함수

	// GetBufferPointer()함수는 버퍼의 시작되는 주소를 보이드 포인터 형으로 반환한다.
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer(); // 보이드 포인터 형이기에 알맞게 형 변환을 해준다.
	m_pMeshMaterials = new D3DMATERIAL9[m_dwNumMaterials]; // 파싱에서 뽑아온 개수만큼 머터리얼 할당
	m_pMeshTextures = new LPDIRECT3DTEXTURE9[m_dwNumMaterials]; // 머터리얼 수만큼 텍스쳐 할당

	// 머터리얼 수 만큼 텍스쳐 로딩
	for (DWORD i = 0; i < m_dwNumMaterials; i++)
	{
		m_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this) Ambient는 사용하지 않아서 색상 값과 같게 설정
		m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;

		m_pMeshTextures[i] = NULL; // 텍스쳐 초기화
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			strlen(d3dxMaterials[i].pTextureFilename) > 0) // 텍스쳐 파일이 존재하면
		{
			D3DXCreateTextureFromFile(m_pd3dDevices, d3dxMaterials[i].pTextureFilename,
				&m_pMeshTextures[i]); // 텍스쳐 파일 설정, bmp파일과 x파일은 함께 있어야 한다.
		}
	}

	pD3DXMtrlBuffer->Release();  // 머티리얼 버퍼 해제

	m_pd3dDevices->SetRenderState(D3DRS_LIGHTING, TRUE);
	m_pd3dDevices->SetRenderState(D3DRS_AMBIENT, 0xffffffff); // 흰색 주변광
	m_pd3dDevices->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void CGameEdu01::OnRender()
{
	// 카메라 행렬 재설정
	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up);
	m_pd3dDevices->SetTransform(D3DTS_VIEW, &m_matView);
	
	for (DWORD i = 0; i < m_dwNumMaterials; i++) // 머터리얼과 그에 해당하는 텍스쳐들 모두 렌더
	{
		m_pd3dDevices->SetMaterial(&m_pMeshMaterials[i]);
		m_pd3dDevices->SetTexture(0, m_pMeshTextures[i]);

		m_pMesh->DrawSubset(i); // 해당 되는 인덱스의 메쉬만 그린다.
	}
}

void CGameEdu01::OnUpdate()
{
	// 카메라 무빙
	if (GetAsyncKeyState('A') < 0)
	{
		m_Eye.x = m_Eye.x - 0.3;
	}
	if (GetAsyncKeyState('D') < 0)
	{
		m_Eye.x = m_Eye.x + 0.3;
	}
	if (GetAsyncKeyState('W') < 0)
	{
		m_Eye.y = m_Eye.y + 0.3;
	}
	if (GetAsyncKeyState('S') < 0)
	{
		m_Eye.y = m_Eye.y - 0.3;
	}
	if (GetAsyncKeyState('Q') < 0)
	{
		m_Eye.z = m_Eye.z + 0.3;
	}
	if (GetAsyncKeyState('E') < 0)
	{
		m_Eye.z = m_Eye.z - 0.3;
	}
}

void CGameEdu01::OnRelease()
{
	if (m_pMeshMaterials != NULL)
	{
		delete[] m_pMeshMaterials;
	}

	if (m_pMeshTextures) // 텍스쳐는 각 개개인의 값을 해제 해주어야 한다.
	{
		for (DWORD i = 0; i < m_dwNumMaterials; i++)
		{
			if (m_pMeshTextures[i])
			{
				m_pMeshTextures[i]->Release();
			}
		}
		delete[] m_pMeshTextures; // 동적 할당 되었던 배열 해제
	}

	if (m_pMesh != NULL)
	{
		m_pMesh->Release();
	}
}