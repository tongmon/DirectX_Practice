// SkullDemo 기반
// 해골 뷰포트 연습
// 4장에서 다뤘던 내용이라 까먹어서 다시 찾아서 봤다.
// OpenGL과 별 다를 것이 없는 뷰포트 설정이다.
// 바뀐 부분은 Class 생성자에 뷰포트 초기화가 있는점,
// DrawScene 부분에 카메라를 바꿔가면서 뷰포트를 변환하는 부분 등이 있다.
// 항상 행렬 계산할 때 월드 * 시야 * 투영 순이라는 것을 잊지말자. 

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"

#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dx11d.lib")
#pragma comment (lib,"D3DCompiler.lib")
#pragma comment (lib,"Effects11d.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"winmm.lib")

struct Vertex // 정점
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class SkullApp : public D3DApp
{
public:
	SkullApp(HINSTANCE hInstance);
	~SkullApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	ID3D11RasterizerState* mWireframeRS;
	D3D11_VIEWPORT vp; // 뷰포트

	// 해골을 국소 공간에서 세계 공간으로 위치시키는 행렬
	XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount; // 해골 인덱스 개수

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SkullApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


SkullApp::SkullApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mVB(0), mIB(0), mFX(0), mTech(0),
	mfxWorldViewProj(0), mInputLayout(0), mWireframeRS(0), mSkullIndexCount(0),
	mTheta(1.5f * MathHelper::Pi), mPhi(0.1f * MathHelper::Pi), mRadius(20.0f)
{
	mMainWndCaption = L"Skull Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	// 처음 뷰포트 전체화면으로 설정
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(mClientWidth);
	vp.Height = static_cast<float>(mClientHeight);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, T);
}

SkullApp::~SkullApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mWireframeRS);
}

bool SkullApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	// 레스터화기 단계
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME; // 뼈대 모드
	wireframeDesc.CullMode = D3D11_CULL_BACK; // 후면 제거
	wireframeDesc.FrontCounterClockwise = false; // 앞면 방향 설정
	wireframeDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));

	return true;
}

void SkullApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void SkullApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void SkullApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dImmediateContext->RSSetState(mWireframeRS);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMVECTOR pos; // 카메라 위치
	XMVECTOR target = XMVectorZero(); // 보는 위치(여기선 고정)
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
	XMMATRIX worldViewProj; // 최종 행렬

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// 왼쪽 상단 해골
		pos = XMVectorSet(0.0f, 0.0f, 20.0f, 1.0f); // 카메라 위치 고정이기에 마우스 안먹힘
		view = XMMatrixLookAtLH(pos, target, up);
		worldViewProj = world * view * proj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = static_cast<float>(mClientWidth) / 2;
		vp.Height = static_cast<float>(mClientHeight) / 2;
		md3dImmediateContext->RSSetViewports(1, &vp); // 뷰포트 설정
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

		// 왼쪽 하단 해골
		pos = XMVectorSet(0.0f, 0.0f, -20.0f, 1.0f); // 카메라 위치 고정이기에 마우스 안먹힘
		view = XMMatrixLookAtLH(pos, target, up);
		worldViewProj = world * view * proj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = static_cast<float>(mClientHeight) / 2;
		vp.Width = static_cast<float>(mClientWidth) / 2;
		vp.Height = static_cast<float>(mClientHeight) / 2;
		md3dImmediateContext->RSSetViewports(1, &vp); // 뷰포트 설정
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

		// 오른쪽 상단 해골
		pos = XMVectorSet(10.0f, 10.0f, 10.0f, 1.0f); // 카메라 위치 고정이기에 마우스 안먹힘
		view = XMMatrixLookAtLH(pos, target, up);
		worldViewProj = world * view * proj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		vp.TopLeftX = static_cast<float>(mClientWidth) / 2;
		vp.TopLeftY = 0.0f;
		vp.Width = static_cast<float>(mClientWidth) / 2;
		vp.Height = static_cast<float>(mClientHeight) / 2;
		md3dImmediateContext->RSSetViewports(1, &vp); // 뷰포트 설정
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

		// 오른쪽 하단 해골
		pos = XMVectorSet(-10.0f, 0.0f, 10.0f, 1.0f); // 카메라 위치 고정이기에 마우스 안먹힘
		view = XMMatrixLookAtLH(pos, target, up);
		worldViewProj = world * view * proj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		vp.TopLeftX = static_cast<float>(mClientWidth) / 2;
		vp.TopLeftY = static_cast<float>(mClientHeight) / 2;
		vp.Width = static_cast<float>(mClientWidth) / 2;
		vp.Height = static_cast<float>(mClientHeight) / 2;
		md3dImmediateContext->RSSetViewports(1, &vp); // 뷰포트 설정
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

void SkullApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void SkullApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SkullApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 5.0f, 50.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SkullApp::BuildGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt"); // 해골의 모델 정점 위치를 읽어 온다.

	if (!fin) // 파일 없을 경우
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore; // string 무시할 때 쓰는 용도

	// 첫 두줄은 정점 개수, 삼각형 개수
	fin >> ignore >> vcount;
	fin >> ignore >> tcount;

	// 그 다음줄은 일단 무시
	fin >> ignore >> ignore >> ignore >> ignore;

	float nx, ny, nz;
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	std::vector<Vertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		// 정점을 txt에서 읽어서 vertices에 담는다.
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;

		vertices[i].Color = black;

		// 법선 벡터는 여기선 안쓴다.
		fin >> nx >> ny >> nz;
	}

	// 무시
	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3 * tcount; // 인덱스는 삼각형 개수의 3배
	std::vector<UINT> indices(mSkullIndexCount); // 인덱스도 정점과 같은 연산
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	// 정점 속성 설정
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	// 인덱스 속성 설정
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

void SkullApp::BuildFX()
{
	std::ifstream fin("fx/color.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, md3dDevice, &mFX));

	mTech = mFX->GetTechniqueByName("ColorTech");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void SkullApp::BuildVertexLayout()
{
	// 입력 배치 설정
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}