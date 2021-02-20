//***************************************************************************************
// PickingDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates picking.
//
// Controls:
//      Click the right mouse button to select.
//
//      Hold '1' for wireframe mode.
//
//***************************************************************************************

// 3D 공간에서 마우스 클릭으로 물체를 더 나아가 물체의 한 폴리곤을 선택하는
// 방법이 나온 예제이다.
// 일단 간단하게 보면 시점에서 레이져를 쏘고 그 레이져에 처음으로 맞은 물체를 선별한다.
// 그 물체의 모든 폴리곤 중에 가장 시점과 가까운 폴리곤이 선택된다.
// 이를 구현하기에 중요한 부분은 일단 레이져를 구축할 수 있어야 하는 것이고
// 나머지는 충돌이다. 정확하게 반직선과 충돌 박스와의 충돌, 삼각 폴리곤과 반직선의 충돌
// 이 두가지를 판정해야 한다.

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "RenderStates.h"
#include "xnacollision.h"

#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dx11d.lib")
#pragma comment (lib,"D3DCompiler.lib")
#pragma comment (lib,"Effects11d.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"winmm.lib")

class PickingApp : public D3DApp
{
public:
	PickingApp(HINSTANCE hInstance);
	~PickingApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildMeshGeometryBuffers();
	void Pick(int sx, int sy);

private:

	ID3D11Buffer* mMeshVB;
	ID3D11Buffer* mMeshIB;

	// 선택된 녀석을 추리기 위해서는 복사본이 필요
	std::vector<Vertex::Basic32> mMeshVertices;
	std::vector<UINT> mMeshIndices;

	XNA::AxisAlignedBox mMeshBox;

	DirectionalLight mDirLights[3];
	Material mMeshMat;
	Material mPickedTriangleMat; // 선택된 녀석은 색상이 다르게

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mMeshWorld;

	UINT mMeshIndexCount;

	UINT mPickedTriangle;

	Camera mCam;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	PickingApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


PickingApp::PickingApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mMeshVB(0), mMeshIB(0), mMeshIndexCount(0), mPickedTriangle(-1)
{
	mMainWndCaption = L"Picking Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	XMMATRIX MeshScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX MeshOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mMeshWorld, XMMatrixMultiply(MeshScale, MeshOffset));

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mMeshMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mMeshMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mMeshMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	mPickedTriangleMat.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

PickingApp::~PickingApp()
{
	ReleaseCOM(mMeshVB);
	ReleaseCOM(mMeshIB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool PickingApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	BuildMeshGeometryBuffers();

	return true;
}

void PickingApp::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void PickingApp::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(10.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-10.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCam.Strafe(-10.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCam.Strafe(10.0f * dt);
}

void PickingApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	mCam.UpdateViewMatrix();

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();




	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());

	ID3DX11EffectTechnique* activeMeshTech = Effects::BasicFX->Light3Tech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeMeshTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the Mesh.

		if (GetAsyncKeyState('1') & 0x8000)
			md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mMeshVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mMeshIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mMeshWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(mMeshMat);

		activeMeshTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mMeshIndexCount, 0, 0); // 전체 자동차를 한번 그리고

		// 레스터화기 단계 복구
		md3dImmediateContext->RSSetState(0);

		//  선택된 삼각형만 다시 다른 재질로 그려준다.

		if (mPickedTriangle != -1)
		{
			// 같은 삼각형을 두번 그리는 것이므로 일반적인 방식인 < 이걸로 비교하면 깊이 판정에서 짤린다.
			// 따라서 <= 로 바꾸어 판정해주게 깊이 스텐실 상태를 바꾸어 주어야 한다.

			md3dImmediateContext->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);

			Effects::BasicFX->SetMaterial(mPickedTriangleMat); // 재질 바꾸고
			activeMeshTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0); // mPickedTriangle 번째의 삼각형만 그린다.

			// 깊이,스텐실 상태 복구
			md3dImmediateContext->OMSetDepthStencilState(0, 0);
		}
	}

	HR(mSwapChain->Present(0, 0));
}

void PickingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(mhMainWnd);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		Pick(x, y); // 우측 클릭으로 선택
	}
}

void PickingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void PickingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void PickingApp::BuildMeshGeometryBuffers()
{
	std::ifstream fin("Models/car.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/car.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
	mMeshVertices.resize(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> mMeshVertices[i].Pos.x >> mMeshVertices[i].Pos.y >> mMeshVertices[i].Pos.z;
		fin >> mMeshVertices[i].Normal.x >> mMeshVertices[i].Normal.y >> mMeshVertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&mMeshVertices[i].Pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	XMStoreFloat3(&mMeshBox.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&mMeshBox.Extents, 0.5f * (vMax - vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mMeshIndexCount = 3 * tcount;
	mMeshIndices.resize(mMeshIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> mMeshIndices[i * 3 + 0] >> mMeshIndices[i * 3 + 1] >> mMeshIndices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &mMeshVertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mMeshVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mMeshIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mMeshIndices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mMeshIB));
}

// 선택 함수
// 레이져를 구축하고 충돌을 검사한다.
// 화면에 찍힌점으로 공간 내의 가상 점을 획득해야한다.
// 과정은 이와 같다.
// 뷰포트 행렬을 NDC좌표 즉 정규화된 좌표계에 위치한 좌표에다 곱해주면
// 실제 모니터에 찍히는 그 좌표가 나온다.
// 이를 반대로 계산하면 실제 모니터에 찍히는 그 좌표로 NDC 좌표를 뜯어낼 수 있다.
// 뜯어낸 NDC 좌표를 실제 좌표계로 만들기 위해 너베에 나누어져 있던 종횡비를 다시 곱한다.
// 그러면 -1 <= x <= 1 --> -r <= x <= r 이렇게 다시 곱하면 가상 공간의 클릭된 좌표가 얻어지는데
// 여기서도 사실 반직선이 다 구해진 것이지만 책에서는 더 나아가 z를 1로 맞추어 줬다.
// 계산이 편하라고 그런건지는 모르겠는데 암튼 z를 1로 맞추어 주려면 z의 현재 값을
// x,y에다가 나누어주면 되는데 이를 표현하여 도출된 최종식이
// 밑에 나온 (+2.0f * sx / mClientWidth - 1.0f) / P(0, 0), -2.0f * sy / mClientHeight + 1.0f) / P(1, 1)
// 이 두 식이다.
void PickingApp::Pick(int sx, int sy)
{
	XMMATRIX P = mCam.Proj();

	// 카메라 좌표계 기준의 반직선 계산
	// 행렬 P의 원소를 사용해서 나누는 것은 
	float vx = (+2.0f * sx / mClientWidth - 1.0f) / P(0, 0);
	float vy = (-2.0f * sy / mClientHeight + 1.0f) / P(1, 1);

	// 시야 공간의 반직선 정의
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 시점, 반직선의 시작은 모두 카메라 원점
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f); // z == 1 거리인 경우 반직선의 방향 벡터

	// 선택 반직선을 물체의 국소 공간으로 맞추어 주어야 한다.
	// 절두체 판별에서와 같은 원리로 좌표계 기준을 맞추어 주어야 올바른 충돌 판정이 가능하다.
	XMMATRIX V = mCam.View();
	XMVECTOR Detv = XMMatrixDeterminant(V);
	XMMATRIX invView = XMMatrixInverse(&Detv, V);

	XMMATRIX W = XMLoadFloat4x4(&mMeshWorld);
	XMVECTOR Detw = XMMatrixDeterminant(W);
	XMMATRIX invWorld = XMMatrixInverse(&Detw, W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld); // 월드 * 시점 이 행렬의 역행렬

	// 점은 XMVector3TransformCoord 이 함수를 사용
	// 벡터는 XMVector3TransformNormal 이 함수를 사용
	// 두 함수의 차이는 w 값이 1이냐 0이냐 기준 차이다.
	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	// 방향은 정규화
	rayDir = XMVector3Normalize(rayDir);

	// 반직선이 메시의 경계상자와 교차하는 경우에만 메시의 삼각형들에 대해
	// 반직선 대 삼각형 교차 판정을 수행한다.
	//
	// 경계상자와 교차하지 않는다면 반직선이 메시와 교차할 가능성이 없으므로,
	// 반직선 대 삼각형 교차 판정으로 시간을 낭비할 이유가 없다.

	// 아직 교차하는 삼각형을 찾지 못했으므로 -1로 초기화
	mPickedTriangle = -1;
	float tmin = 0.0f;
	if (XNA::IntersectRayAxisAlignedBox(rayOrigin, rayDir, &mMeshBox, &tmin)) // 반직선과 AABB가 충돌했다면
	{
		// 반직선과 교차하는 가장 가까운 삼각형을 찾는다.
		tmin = MathHelper::Infinity;
		for (UINT i = 0; i < mMeshIndices.size() / 3; ++i)
		{
			// 삼각형의 색인
			UINT i0 = mMeshIndices[i * 3 + 0];
			UINT i1 = mMeshIndices[i * 3 + 1];
			UINT i2 = mMeshIndices[i * 3 + 2];

			// 색인을 통해 정점 획득
			XMVECTOR v0 = XMLoadFloat3(&mMeshVertices[i0].Pos);
			XMVECTOR v1 = XMLoadFloat3(&mMeshVertices[i1].Pos);
			XMVECTOR v2 = XMLoadFloat3(&mMeshVertices[i2].Pos);

			// 교차하는 가장 가까운 삼각형을 찾기위해서 메시의 모든 삼각형 판정
			float t = 0.0f;
			if (XNA::IntersectRayTriangle(rayOrigin, rayDir, v0, v1, v2, &t))
			{
				if (t < tmin) // 제일 적은 거리를 찾음
				{
					// 이 삼각형이 현재까지 가장 가까운 삼각형이다.
					tmin = t;
					mPickedTriangle = i;
				}
			}
		}
	}
}