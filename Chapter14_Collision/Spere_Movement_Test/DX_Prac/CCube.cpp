#include "CCube.h"
#include <d3dx9math.h>

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The transformed position for the vertex.
	DWORD color;        // The vertex color.
};

CCube::CCube(void)
{
	m_pVB = NULL;
	m_pIB = NULL;
}

CCube::~CCube(void)
{
}

void CCube::OnInit(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice;

	// 정육면체 정점 8개 선언
	CUSTOMVERTEX Vertices[] = {
		{ D3DXVECTOR3(-1.0f, 1.0f,  1.0f) ,  D3DCOLOR_RGBA(255, 0, 255, 255) },
		{ D3DXVECTOR3(1.0f, 1.0f,  1.0f)  , D3DCOLOR_RGBA(0, 255, 0, 255) },
		{ D3DXVECTOR3(1.0f, 1.0f, -1.0f)  , D3DCOLOR_RGBA(0, 0, 255, 255) },
		{ D3DXVECTOR3(-1.0f, 1.0f, -1.0f)  , D3DCOLOR_RGBA(255, 255, 0, 255) },

		{ D3DXVECTOR3(-1.0f, -1.0f,  1.0f) , D3DCOLOR_RGBA(255, 0, 0, 255) },
		{ D3DXVECTOR3(1.0f, -1.0f,  1.0f) , D3DCOLOR_RGBA(0, 255, 10, 255) },
		{ D3DXVECTOR3(1.0f, -1.0f, -1.0f) , D3DCOLOR_RGBA(0, 0, 255, 255) },
		{ D3DXVECTOR3(-1.0f, -1.0f, -1.0f) , D3DCOLOR_RGBA(255, 0, 0, 255) }
	};

	// Note: 인덱스 정보, 위 Vertices에서 정의한 정점들을 시계방향으로 삼각형 단위로 묶는 인덱스들을 선언한다.
	// 정육면체는 사각형이 6개이고 사각형 1개는 삼각형 2개로 이루어지기에 인덱스는 12개다.
	// 인덱스를 쓰는 이유는 정점으로 출력을 하게 될 시에 삼각형 한 개 출력에 정점 3개가 필요한데 이게
	// 정육면체 출력시에 중복이 되면서 36개로 커지게 된다.
	// 이를 인덱스 버퍼를 사용하면 12개로 줄일 수가 있기에 인덱스 버퍼를 사용한다.
	WORD Indices[] =
	{
			0, 1, 2,
			0, 2, 3,
			4, 6, 5,
			4, 7, 6,
			0, 3, 7,
			0, 7, 4,
			1, 5, 6,
			1, 6, 2,
			3, 2, 6,
			3, 6, 7,
			0, 4, 5,
			0, 5, 1
	};

	// 정점 버퍼 생성 및 설정
	m_pd3dDevice->CreateVertexBuffer(sizeof(Vertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL);

	void* pVertices;
	m_pVB->Lock(0, sizeof(Vertices), &pVertices, 0);
	memcpy(pVertices, Vertices, sizeof(Vertices));
	m_pVB->Unlock();

	// 인덱스 버퍼 생성 및 설정	
	m_pd3dDevice->CreateIndexBuffer(sizeof(Indices), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL);

	void* pIndices;
	m_pIB->Lock(0, sizeof(Indices), &pIndices, 0);
	memcpy(pIndices, Indices, sizeof(Indices));
	m_pIB->Unlock();
}

void CCube::OnRender()
{
	// 와이어 프레임 출력 모드, 도형의 뼈대만 모인다. 
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	// 컬링이 들어간 부분은 원래 화면에 출력되지 않는데 이를 무시하고 다 출력해주는 모드
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	
	// 그대로 출력하기 위해 단위행렬 얻기
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);

	// 면에 대한 법선 벡터를 구하지 않았기 때문에 라이트를 꺼 원래 색상을 출력한다.
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// 원래 좌표와 같게 물체 설정
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	// 정점 버퍼에 있는 정점들 출력 설정
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
	m_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	// 정점 인덱스도 버퍼와 마찬가지로 출력 설정을 해준다.
	m_pd3dDevice->SetIndices(m_pIB);
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 6 * 2); // sizeof( Indices ) / sizeof(WORD) / 3

	// 라이팅을 원래대로
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	// 컬링 출력 상태 원래대로
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CCube::OnRelease()
{
	if (m_pVB != NULL)
		m_pVB->Release();

	if (m_pIB != NULL)
		m_pIB->Release();
}