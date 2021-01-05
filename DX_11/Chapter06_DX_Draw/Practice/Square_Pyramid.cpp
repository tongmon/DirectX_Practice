// BoxDemo 기반
// 사면체 만드는 연습
// 사면체 정점들을 노트에 그려서 그대로 옮기면 별로 어렵지 않다.
// 유의할 점은 밑면은 앞면을 시계방향으로 설정했다면
// 반시계방향이라는 것이다. 
// 위에서 보면 밑면은 후면이니까.
// 바뀐 부분은 BuildGeometryBuffers 함수 부분, 인덱스 개수가 18개가 된 것 정도.

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"

#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dx11d.lib")
#pragma comment (lib,"D3DCompiler.lib")
#pragma comment (lib,"Effects11d.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"winmm.lib")

struct Vertex // 정점 정의
{
	XMFLOAT3 Pos; // 위치
	XMFLOAT4 Color; // 색상
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

	bool Init(); // 각종 변수들 초기화
	void OnResize(); // 창 갱신
	void UpdateScene(float dt); // 화면 갱신
	void DrawScene(); // 그리기

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers(); // 정점 정의 함수
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mBoxVB; // 정점 버퍼
	ID3D11Buffer* mBoxIB; // 인덱스 버퍼

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld; // 월드 행렬, SRT
	XMFLOAT4X4 mView; // 카메라 위치
	XMFLOAT4X4 mProj; // 투영 행렬

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// 디버그 빌드 모드에서 실행시점 메모리 점검 기능 활성화
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BoxApp theApp(hInstance);

	if (!theApp.Init()) // 초기화
		return 0;

	return theApp.Run(); // 이벤트 루프 작동
}

BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mFX(0), mTech(0),
	mfxWorldViewProj(0), mInputLayout(0),
	mTheta(1.5f * MathHelper::Pi), mPhi(0.25f * MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Box Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	// XMStoreFloat4x4 ---> XMMATRIX 형을 XMFloat4x4 형에 저장
	XMMATRIX I = XMMatrixIdentity(); // 단위 행렬로 초기화
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

BoxApp::~BoxApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
}

bool BoxApp::Init() // 초기화
{
	if (!D3DApp::Init()) // D3DApp에서 초기화에 관련 함수가 있다.
		return false;

	BuildGeometryBuffers(); // 정점 정의
	BuildFX(); // FX 파일 읽어서 쉐이딩
	BuildVertexLayout(); // 입력 배치 설정 함수

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// 창 크기 변경에 따라 종횡비 갱신, 다시 계산 --> AspectRatio()
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f); // 투영행렬 설정
	XMStoreFloat4x4(&mProj, P); // 투영 행렬 갱신
}

void BoxApp::UpdateScene(float dt)
{
	// 구면 좌표계는 (r,θ,φ)로 표현
	// r = 반지름
	// θ = y축의 양의 방향으로부터 원점과 반지름 막대가 이루는 직선까지의 각
	// φ = x축의 양의 방향으로부터 원점과 반지름 막대가 이루는 직선을 xz평면에 투영시킨 직선까지의 각
	// mRadius = r, mPhi = θ, mTheta = φ
	// 즉 좌표계가 둥그런 구와 같고 그 안에서 반지름과 각으로 좌표를 표현하게 된다.
	// 이를 직교 좌표계로 변환하기 위해 밑과 같은 공식을 사용한다.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// 시야 행렬 구축
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f); // 카메라 위치, 좌표니까 1.0f
	XMVECTOR target = XMVectorZero(); // 원점을 바라봄
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 보통 카메라 기울기 설정(거의 고정)

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up); // 시야행렬 설정
	XMStoreFloat4x4(&mView, V); // mView = 시야 행렬
}

void BoxApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue)); // 렌더 버퍼 설정
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 설정

	md3dImmediateContext->IASetInputLayout(mInputLayout); // 입력 배치를 사용하고자 하는 장치에 묶기
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 삼각 리스트 형태의 방식으로

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0); // 인덱스 버퍼들을 입력 조립기 단계에 묶기

	// 변환 상수들 설정
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// 36개의 박스 인덱스 버퍼들
		md3dImmediateContext->DrawIndexed(18, 0, 0); // 정점 인덱스 개수, 첫 정점 위치, 정점에 더해질 값을 지정해 실제로 그리기
	}

	HR(mSwapChain->Present(0, 0)); // 더블 버퍼링의 스왑 체인
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y) // 마우스 버튼 누를때
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y) // 마우스 버튼 땔 때
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0) // 좌측 클릭으로 카메라 위치 이동
	{
		// 1픽셀이 4분의 1도 (Degree 단위) 가 되게 한다.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// 마우스 입력에 기초한 각도로 상자 주변의 궤도 카메라 갱신
		mTheta += dx;
		mPhi += dy;

		// mPhi(사람 시점으로는 상하 고개 끄덕이는 거)의 각도 범위를 0 ~ 180으로 제한하는 것.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0) // 우측 클릭으로 카메라 확대 축소
	{
		// 1픽셀이 장면의 0.005 단위가 되게 한다.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// 마우스 입력에 기초하여 궤도 카메라의 반지름을 조정
		mRadius += dx - dy;

		// 반지름의 범위도 3 ~ 15로 제한을 둔다.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BoxApp::BuildGeometryBuffers() // 정점 정의 함수
{
	// 정점들 정의, 색상은 d3dUtil.h에 정의됨
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), (const float*)&Colors::Green   },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), (const float*)&Colors::Green   },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), (const float*)&Colors::Green     },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), (const float*)&Colors::Green   },
		{ XMFLOAT3(0.0f, +1.0f, 0.0f), (const float*)&Colors::Red    }
	};

	D3D11_BUFFER_DESC vbd;
	// 보통은 D3D11_USAGE_DEFAULT를 해서 GPU가 자원을 읽고 쓰게 하지만 
	// 여기선 최적화를 위해 버퍼 생성 후에 전혀 변경이 없는 D3D11_USAGE_IMMUTABLE를 사용
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 5; // 생성할 정점 버퍼의 크기
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 정점 버퍼는 무조건 이거 쓴다.
	vbd.CPUAccessFlags = 0; // CPU가 정점 버퍼 계산에 관여하지 않으므로 0
	vbd.MiscFlags = 0; // 정점 버퍼에서는 이건 무조건 0
	vbd.StructureByteStride = 0; // 구조적 버퍼 사용 안하면 0
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB)); // vbd(정점 버퍼 속성), vinitData(정점 초기화), mBoxVB(생성된 정점이 담김)

	// 인덱스 버퍼 생성, 시계방향이 DX에서 앞면
	UINT indices[] = {
		// 밑면
		0, 2, 3,
		0, 1, 2,

		// 옆면
		0, 4, 1,
		1, 4, 2,
		2, 4, 3,
		3, 4, 0
	};

	// 인덱스 버퍼 특성 지정, 방식은 정점 버퍼와 비슷
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 18; // 36개의 인덱스
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; // 인덱스 버퍼는 이렇게 지정
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData; // 색인 버퍼 초기화할 자료를 지정
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB)); // 색인 버퍼 생성, HR은 디버그 용도
}

void BoxApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(L"FX/color.fx", 0, 0, 0, "fx_5_0", shaderFlags,
		0, 0, &compiledShader, &compilationMsgs, 0);

	// compilationMsgs에 오류 메시지나 경고 메시지가 저장되어 있을 수 있다.
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	// compilationMsgs에 오류 메시지가 없다고 해도 명시적으로 컴파일 오류 여부를 검사
	if (FAILED(hr))
	{
		DXTrace((WCHAR*)__FILEW__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, md3dDevice, &mFX)); // 이펙트를 mFX에 저장

	// 컴파일된 셰이더를 다 사용해서 해제함
	ReleaseCOM(compiledShader);

	mTech = mFX->GetTechniqueByName("ColorTech"); // fx 파일을 보면 ColorTech라는 이름의 테크님 구조체가 있다. 이를 가져오는 것
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix(); // fx 파일에서 gWorldViewProj 변수를 행렬 형태로 획득함
}

void BoxApp::BuildVertexLayout()
{
	// 정점 입력 배치 만들기
	// 입력 데이터의 유형을 알아야 한다.
	// 성분에 부여될 이름, 색인(보통은 0이지만 하나의 정점에 입혀질 텍스쳐가 여러개라면 색인이 유효해진다.)
	// , 정점의 자료 형식(예로 DXGI_FORMAT_R32G32B32_FLOAT는 3차원 32비트 부동소수점을 의미)
	// , 입력 슬롯 개수(위치와 색상을 따로 쪼개서 보내려 한다면 사용)
	// , 바이트 단위의 오프셋(색상 특성은 위치가 3개의 float형이라 12부터 시작하여 12로 지정)
	// , 나머지 2개의 속성은 일단 고정
	// 자세한 내용은 책 212p 참조
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// 입력 배치 객체 생성
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout)); // 생성된 입력 배치는 mInputLayout 여기에 담기게 된다.
}