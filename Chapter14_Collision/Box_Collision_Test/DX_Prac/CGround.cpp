#include "CGround.h"

struct GROUNDVERTEX // 정점의 좌표와 색을 묶음
{
	D3DXVECTOR3 vPos;
	DWORD		dwDiffuse;
};

#define D3DFVF_GROUNDVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE) // 정점 포맷 설정

CGround::CGround(void)
{

}


CGround::~CGround(void)
{

}

// 디바이스의 포인터와 행의 개수, 열의 개수, 정점 간의 간격을 받는 지면 생성 함수 (여기서 행렬의 기준은 정점이 아닌 격자 기준이다.)
void CGround::Create(LPDIRECT3DDEVICE9 pd3dDevice, int nRow, int nCol, float fSize)
{
	m_nCol = nCol;
	m_nRow = nRow;

	// 정점의 개수는 격자 기준의 행렬에서 각 +1을 한 후 곱한 것과 같다. 2행 3열의 격자는 6개지만 이때 정점의 수는 12개다.
	m_dwNumVertices = (m_nCol + 1) * (m_nRow + 1); 
	// 삼각형 2개가 만나야 4각형의 격자를 이룬다. 삼각형의 정점은 3개이고 이런게 2개가 만나기에 3 + 3 = 6 이 된다.
	m_dwNumIndices = m_nCol * m_nRow * 6;
	m_pd3dDevice = pd3dDevice;

	GROUNDVERTEX* pGroundVertex = new GROUNDVERTEX[m_dwNumVertices];

	// Note: 시작 위치
	int nIndex = 0; // 실제 격자는 2차원 배열 처럼 표현되지만 정점을 담는 배열은 일차원으로 나타낸다.
	// x좌표가 밑과 같은 이유는 원점에서 맵 전체 크기의 절반이 음수 쪽으로 가야해서 그렇고 z좌표는 음수가 아니라 양수 쪽으로 가야해서 밑과 같이 표현된다.
	// 즉 시작 좌표를 정하는 것이다. 오른쪽 아래로 가면서 정점들이 찍힌다고 보면 편하다. 
	D3DXVECTOR3 vPos0(-1.0f * m_nCol * fSize * 0.5f, 0.0f, m_nRow * fSize * 0.5f);
	for (int z = 0; z <= m_nRow; z++)
	{
		for (int x = 0; x <= m_nCol; x++)
		{
			pGroundVertex[nIndex].vPos.x = vPos0.x + (fSize * x);
			pGroundVertex[nIndex].vPos.y = 0.0f;
			pGroundVertex[nIndex].vPos.z = vPos0.z + -1.0f * (fSize * z); // z는 애초에 양수여서 아래로 이동하기 위해 빼준다.
			pGroundVertex[nIndex].dwDiffuse = D3DCOLOR_RGBA(255, 50, 255, 255);
			nIndex++;
		}
	}

	//Note: 버텍스 버퍼 생성 
	if (m_pd3dDevice->CreateVertexBuffer(m_dwNumVertices * sizeof(GROUNDVERTEX), 0, D3DFVF_GROUNDVERTEX, D3DPOOL_DEFAULT, &m_pVB, 0) != D3D_OK)
	{
		MessageBox(NULL, "정정 버퍼 생성 Error", "Error", MB_OK);
		return;
	}

	void* pVertices;
	if (m_pVB->Lock(0, 0, &pVertices, NULL) != D3D_OK) // 2번째 매개변수는 전체를 lock하는 값 0	
	{
		MessageBox(NULL, "정정 버퍼 lock Error", "Error", MB_OK);
		return;
	}

	memcpy(pVertices, pGroundVertex, m_dwNumVertices * sizeof(GROUNDVERTEX));
	m_pVB->Unlock();

	//Note: 인덱스 버퍼 생성
	WORD* pIndex = new WORD[m_dwNumIndices];
	nIndex = 0;
	// pIndex의 값을 결정하는 부분은 이차원 배열을 일차원으로 담는 경우로 한번 격자를 찍어서 계산해보면 이해하기 쉽다. 그리고 항상 인덱스는 CW방향이다.
	for (int z = 0; z < m_nRow; z++)
	{
		for (int x = 0; x < m_nCol; x++)
		{
			pIndex[nIndex++] = WORD(z * (m_nCol + 1) + x);
			pIndex[nIndex++] = WORD((z + 1) * (m_nCol + 1) + x + 1);
			pIndex[nIndex++] = WORD((z + 1) * (m_nCol + 1) + x);

			pIndex[nIndex++] = WORD(z * (m_nCol + 1) + x);
			pIndex[nIndex++] = WORD(z * (m_nCol + 1) + x + 1);
			pIndex[nIndex++] = WORD((z + 1) * (m_nCol + 1) + x + 1);
		}
	}

	void* pIndices;
	m_pd3dDevice->CreateIndexBuffer(m_dwNumIndices * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL);
	m_pIB->Lock(0, 0, (void**)&pIndices, 0);
	memcpy(pIndices, pIndex, sizeof(WORD) * m_dwNumIndices);
	m_pIB->Unlock();

	delete[] pIndex;
	delete[] pGroundVertex;
}

void CGround::OnRender()
{
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);

	// 출력을 위한 불끄기와 뼈대 보여주기 모드
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(GROUNDVERTEX));
	m_pd3dDevice->SetIndices(m_pIB);
	m_pd3dDevice->SetFVF(D3DFVF_GROUNDVERTEX);
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_dwNumVertices, 0, m_dwNumIndices / 3);
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
}

void CGround::OnRelease()
{
	m_pIB->Release();
	m_pVB->Release();
}