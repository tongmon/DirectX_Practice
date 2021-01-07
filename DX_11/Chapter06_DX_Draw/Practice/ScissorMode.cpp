// ShapeDemo 기반
// 레스터화기 단계에서 가위모드를 지정할 수 있다.
// 이를 해보니 슈퍼마리오카트에서 먹물 뿌리면 가려지는 효과 등에 쓰면
// 제격일 것 같다는 생각이 든다.
// 바뀐 부분은 Init 함수에서 wireframeDesc.ScissorEnable = true; 부분,
// DrawScene 함수에서 사각형 크기 정하고 가위 옵션에 적용해주는 부분 등이 있다.
// 딱 그 사각형 안에만 보인다...

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
	XMFLOAT3 Pos; // 위치
	XMFLOAT4 Color; // 색상
};

class ShapesApp : public D3DApp
{
public:
	ShapesApp(HINSTANCE hInstance);
	~ShapesApp();

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

	// 국소 공간에서 세계 공간으로의 변환 행렬을 정의한다.
	XMFLOAT4X4 mSphereWorld[10]; // 구 10개 그릴거
	XMFLOAT4X4 mCylWorld[10]; // 원기둥 10개 그릴거
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	XMFLOAT4X4 mView; // 시점
	XMFLOAT4X4 mProj; // 투영

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

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

	ShapesApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


ShapesApp::ShapesApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mVB(0), mIB(0), mFX(0), mTech(0),
	mfxWorldViewProj(0), mInputLayout(0), mWireframeRS(0),
	mTheta(1.5f * MathHelper::Pi), mPhi(0.1f * MathHelper::Pi), mRadius(15.0f)
{
	mMainWndCaption = L"Shapes Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	// 단위 행렬로 초기화
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f); // 박스 크기 조절
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f); // 박스 위치 조정
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset)); // S, T 연산 행렬

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&mCenterSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	// 원기둥들은 다섯줄로 배치됨, 하나의 줄은 두 개의 원기둥+구 쌍으로 이루어 진다.
	// x 위치만 마주보는 반전으로 배치라 10번 할 필요없이 5번 루프 돌리면 된다.
	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}
}

ShapesApp::~ShapesApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mWireframeRS);
}

bool ShapesApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers(); // 정점, 인덱스 정의
	BuildFX(); // 쉐이더
	BuildVertexLayout(); // 입력 배치

	D3D11_RASTERIZER_DESC wireframeDesc; // 레스터화기 상태 집합을 서술
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME; // 와이어 프레임
	wireframeDesc.CullMode = D3D11_CULL_BACK; // 후면 제거
	wireframeDesc.FrontCounterClockwise = false; // 앞면을 시계 방향 정의된 것으로
	wireframeDesc.DepthClipEnable = true;
	wireframeDesc.ScissorEnable = true; // 가위 모드 ON, 정해진 사각형 밖의 모든 픽셀은 폐기

	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));

	return true;
}

void ShapesApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void ShapesApp::UpdateScene(float dt)
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

void ShapesApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 가위 모드를 적용할 사각형 지정, 여러개 지정할 수 있다.
	D3D11_RECT rect = { 100,100,400,400 };
	md3dImmediateContext->RSSetScissorRects(1, &rect);

	md3dImmediateContext->RSSetState(mWireframeRS); // 래스터화기 설정 적용
	// md3dImmediateContext->RSSetState(0); // 래스터화기 설정 기본으로 되돌림

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	// 상수들 설정

	XMMATRIX view = XMLoadFloat4x4(&mView); // 시야
	XMMATRIX proj = XMLoadFloat4x4(&mProj); // 투영
	XMMATRIX viewProj = view * proj; // 시야, 투영 통합

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		XMMATRIX buf; // reinterpret_cast<float*>의 l-value 문제 해결을 위해 임시 저장 변수 사용
		
		// 격자 그리기
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);		
		buf = world * viewProj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&buf)); // 월드 변환, 뷰 변환을 합친 최종 행렬로 위치 설정
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		// 격자의 인덱스 수, 격자 인덱스 시작 위치, 격자 정점의 시작 부분 ---> 이렇게 3개를 알아야 통합 인덱스 리스트를 참조하여 원활히 그려짐
		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// 상자 그리기
		world = XMLoadFloat4x4(&mBoxWorld);
		buf = world * viewProj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&buf));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// 구 그리기
		world = XMLoadFloat4x4(&mCenterSphere);
		buf = world * viewProj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&buf));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

		// 원기둥 그리기
		for (int i = 0; i < 10; ++i) // 10개 그려야한다.
		{
			world = XMLoadFloat4x4(&mCylWorld[i]); // 저장해둔 S,R 행렬 mCylWorld[i] 획득
			buf = world * viewProj;
			mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&buf));
			mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// 구 그리기
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			buf = world * viewProj;
			mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&buf));
			mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	HR(mSwapChain->Present(0, 0));
}

void ShapesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void ShapesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void ShapesApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 200.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void ShapesApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	// 각종 도형들 정점, 인덱스 배열 생성해서 MeshData에 담음
	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	//geoGen.CreateGeosphere(0.5f, 2, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// 연결된 정점 버퍼 안에서 각 물체의 정점 오프셋을 저장해 둔다.
	mBoxVertexOffset = 0; // 처음 생성한 박스는 처음 부터
	mGridVertexOffset = box.Vertices.size(); // 박스 이후니까 박스 정점 후에 등장
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size(); // 박스와 격자 후에 등장
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size(); // 박스와 격자와 구 후에 등장

	// 각 물체의 인덱스 개수를 저장해 둔다.
	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// 연결된 인덱스 버퍼에서의 각 물체의 시작 부분을 저장해 둔다. 정점과 같은 원리
	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount = // 전체 정점 개수
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount = // 전체 인덱스 개수
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	// 필요한 정점들을 추출하고, 모든 메시의 정점들을
	// 하나의 버퍼에 넣는다.
	//

	/*
	| 박스 인덱스 | 지면 인덱스 | 구 인덱스 | 원기둥 인덱스 |
	정점도 마찬가지
	*/

	std::vector<Vertex> vertices(totalVertexCount);

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f); // 색상은 검은색으로

	UINT k = 0; // 통합 정점을 위한 색인
	// 박스를 통합 정점에 삽입
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Color = black;
	}
	// 격자를 통합 정점에 삽입
	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Color = black;
	}
	// 구를 통합 정점에 삽입
	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Color = black;
	}
	// 원기둥을 통합 정점에 삽입
	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Color = black;
	}

	// 정점 속성들 지정
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	//
	// 모든 메시의 색인들을 하나의 색인 버퍼에 넣음
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	// 인덱스 속성들 지정
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

void ShapesApp::BuildFX()
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

void ShapesApp::BuildVertexLayout()
{
	// 입력 배치 정의
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