//***************************************************************************************
// InstancingAndCullingDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates hardware instancing and frustum culling.  Only objects intersecting
// the frustum are instanced and drawn.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//      '1' Turn on frustum culling.
//      '2' Turn off frustum culling.
//
//***************************************************************************************

// 이 예제는 인스턴싱에 대해 다루는데 인스턴싱이 뭐냐면
// 만약 어떤 한 물체를 다양한 위치에 다양한 색깔로 다양한 텍스쳐를 입혀
// 그리려 한다면 일반적으로 인스턴싱을 모르고 이를 행하려하면 배열을 만들어서
// 그리기를 다양한 물체의 수 만큼 진행을 할 것이다.
// 근데 이러면 그리기 Api 호출을 여러번 하게 되고 이는 속도 저하의 원인이 된다.
// 따라서 이러한 배열을 따로 두지않고 인스턴싱을 사용하여 한번의 그리기로
// 같은 물체를 다양하게 그릴 수가 있다.

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "xnacollision.h"

#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dx11d.lib")
#pragma comment (lib,"D3DCompiler.lib")
#pragma comment (lib,"Effects11d.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"winmm.lib")

// 인스턴싱 데이터
struct InstancedData
{
	XMFLOAT4X4 World;
	XMFLOAT4 Color;
};

class InstancingAndCullingApp : public D3DApp 
{
public:
	InstancingAndCullingApp(HINSTANCE hInstance);
	~InstancingAndCullingApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildSkullGeometryBuffers();
	void BuildInstancedBuffer();

private:
	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;
	ID3D11Buffer* mInstancedBuffer;

	// Bounding box of the skull.
	XNA::AxisAlignedBox mSkullBox;
	XNA::Frustum mCamFrustum; // 카메라 중심 좌표계인 경우 절두체
	
	UINT mVisibleObjectCount;

	// 절두체 선별전 모든 물체 정보
	std::vector<InstancedData> mInstancedData;

	bool mFrustumCullingEnabled;

	DirectionalLight mDirLights[3];
	Material mSkullMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount;

	Camera mCam;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	InstancingAndCullingApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}
 

InstancingAndCullingApp::InstancingAndCullingApp(HINSTANCE hInstance)
: D3DApp(hInstance), mSkullVB(0), mSkullIB(0), mSkullIndexCount(0), mInstancedBuffer(0),
  mVisibleObjectCount(0), mFrustumCullingEnabled(true)
{
	mMainWndCaption = L"Instancing and Culling Demo";
	
	srand((unsigned int)time((time_t *)NULL));

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	XMMATRIX I = XMMatrixIdentity();

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));


	mDirLights[0].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mSkullMat.Ambient  = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mSkullMat.Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

InstancingAndCullingApp::~InstancingAndCullingApp()
{
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mInstancedBuffer);

	Effects::DestroyAll();
	InputLayouts::DestroyAll(); 
}

bool InstancingAndCullingApp::Init()
{
	if(!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	BuildSkullGeometryBuffers();
	BuildInstancedBuffer();

	return true;
}

void InstancingAndCullingApp::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	// Build the frustum from the projection matrix in view space.
	XMMATRIX Proj = mCam.Proj();
	ComputeFrustumFromProjection(&mCamFrustum, &Proj);
}

void InstancingAndCullingApp::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
	if( GetAsyncKeyState('W') & 0x8000 )
		mCam.Walk(10.0f*dt);

	if( GetAsyncKeyState('S') & 0x8000 )
		mCam.Walk(-10.0f*dt);

	if( GetAsyncKeyState('A') & 0x8000 )
		mCam.Strafe(-10.0f*dt);

	if( GetAsyncKeyState('D') & 0x8000 )
		mCam.Strafe(10.0f*dt);
		
	if( GetAsyncKeyState('1') & 0x8000 )
		mFrustumCullingEnabled = true;

	if( GetAsyncKeyState('2') & 0x8000 )
		mFrustumCullingEnabled = false;

	//
	// 절두체 선별
	//

	mCam.UpdateViewMatrix();
	mVisibleObjectCount = 0;

	// 절두체 선별이 켜진 상태면
	if(mFrustumCullingEnabled)
	{
		XMVECTOR detView = XMMatrixDeterminant(mCam.View());
		XMMATRIX invView = XMMatrixInverse(&detView, mCam.View()); // 시점 행렬 역행렬 획득
	
		// 인스턴스 버퍼에 자료를 기록하기 위해 시스템 메모리에 매핑한다.
		D3D11_MAPPED_SUBRESOURCE mappedData; 
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData); // 덮어쓰기 허용

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for(UINT i = 0; i < mInstancedData.size(); ++i)
		{
			XMMATRIX W = XMLoadFloat4x4(&mInstancedData[i].World);
			XMVECTOR Det = XMMatrixDeterminant(W);
			XMMATRIX invWorld = XMMatrixInverse(&Det, W);

			// 시야 공간에서 물체의 국소 공간으로 변환
			// 왜 이 짓을 하냐면 현재 해당 물체는 세계 변환 행렬이 곱해져 있지 않고
			// 자신의 국소 공간에 위치된 상태이다.
			// 근데 카메라 기준 좌표계로 설정되어 있는 절두체는 World * View 행렬이 곱해져 있는 상태와 같다.
			// 더 자세하게 보면 물체는 국소 공간에서 세계 좌표계 기준으로 표현되려고 World 행렬이 곱해지고
			// 다시 판정이 편하게 세계 좌표계에서 카메라 기준 좌표계로 변하게 View 행렬이 곱해지는 것이다.
			// 현재 mInstancedData에 담긴 물체는 그냥 World, View 이런 행렬이 곱해지기 전 상태이고
			// 따라서 물체는 국소 공간에 있는 것이다.
			// 절두체도 물체와 좌표계를 똑같이 맞추어 올바른 절두체 판정이 되게 해주어야 한다.
			// 예를 들면 국소 공간에서 (0,0,12)는 카메라 좌표계에서는 (1,23,0) 이렇게 표현이 될 수도 있는 것이라
			// 올바른 판정을 하기위해 좌표계를 똑같이 맞추어 줘야 하는 것이다.
			// 여기서는 국소 공간으로 절두체를 옮겨 주었지만 반대로 물체를 카메라 기준 좌표계로 옮겨 주어도 된다.
			// 변환 행렬 * 시야 행렬 여기에 (변환 행렬 * 시야 행렬)의 역행렬을 곱하면
			// 물체의 국소 공간이 획득된다.
			XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);
		
			// (변환 행렬 * 시야 행렬)의 역행렬을 개별 변환들로 분해한다.
			XMVECTOR scale;
			XMVECTOR rotQuat;
			XMVECTOR translation;
			XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);

			// 카메라 절두체를 시야 공간에서 물체의 국소 공간으로 변환한다.
			XNA::Frustum localspaceFrustum;
			XNA::TransformFrustum(&localspaceFrustum, &mCamFrustum, XMVectorGetX(scale), rotQuat, translation);

			// 국소 공간에서 절두체 평면과 물체의 절단 여부를 가려냄
			if(XNA::IntersectAxisAlignedBoxFrustum(&mSkullBox, &localspaceFrustum) != 0)
			{
				// dataView 동적 버퍼에 절단이 필요없이 그려져야하는 물체를 넣기
				// 결국 mInstancedBuffer에는 mVisibleObjectCount 만큼의 그리기 유효한
				// 물체가 담기게 된다.
				dataView[mVisibleObjectCount++] = mInstancedData[i];
			}
		}

		md3dImmediateContext->Unmap(mInstancedBuffer, 0);
	}
	else // No culling enabled, draw all objects.
	{
		D3D11_MAPPED_SUBRESOURCE mappedData; 
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for(UINT i = 0; i < mInstancedData.size(); ++i)
		{
			dataView[mVisibleObjectCount++] = mInstancedData[i];
		}

		md3dImmediateContext->Unmap(mInstancedBuffer, 0);
	}

	std::wostringstream outs;   
	outs.precision(6);
	outs << L"Instancing and Culling Demo" << 
		L"    " << mVisibleObjectCount << 
		L" objects visible out of " << mInstancedData.size();
	mMainWndCaption = outs.str();
}

void InstancingAndCullingApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::InstancedBasic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride[2] = {sizeof(Vertex::Basic32), sizeof(InstancedData)};
    UINT offset[2] = {0,0};

	ID3D11Buffer* vbs[2] = {mSkullVB, mInstancedBuffer};
 
	XMMATRIX view     = mCam.View();
	XMMATRIX proj     = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	// Set per frame constants.
	Effects::InstancedBasicFX->SetDirLights(mDirLights);
	Effects::InstancedBasicFX->SetEyePosW(mCam.GetPosition());
 
	ID3DX11EffectTechnique* activeTech = Effects::InstancedBasicFX->Light3Tech;
	 
    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the skull.

		md3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

		Effects::InstancedBasicFX->SetWorld(world);
		Effects::InstancedBasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::InstancedBasicFX->SetViewProj(viewProj);
		Effects::InstancedBasicFX->SetMaterial(mSkullMat);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		// 인스턴싱을 사용한 그리기를 할 때는 DrawIndexedInstanced 함수를 이용한다.
		// 첫번째 인자는 인덱스 개수, 두번째는 인스턴스 개수, 나머지 2개는 인덱스 그리는 것과 같고
		// 마지막 인자는 인스턴스 첫 원소 색인이다.
		// 절두체 선별에서 빠지는 물체들을 제외한 유효한 물체의 개수인 mVisibleObjectCount 만큼 그린다.
		md3dImmediateContext->DrawIndexedInstanced(mSkullIndexCount, mVisibleObjectCount, 0, 0, 0);
	}
	
	HR(mSwapChain->Present(0, 0));
}

void InstancingAndCullingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void InstancingAndCullingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void InstancingAndCullingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
 
void InstancingAndCullingApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");
	
	if(!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;
	
	// AABB 구하기, AABB는 어떤 물체를 감쌀 수 있는 최소 박스 경계를 말한다.
	// AABB는 그리고 물체가 축에 정렬되어 있다는 가정하에 진행된다.
	// 축에 정렬이 안되어있는 상태에서 하면 물체보다 뚱뚱한 돼지 상자가 만들어질 수가 있고
	// 충돌 판정이 이상해질 것이다.
	// 정렬 안된 상태에서는 OBB를 써서하는데 이는 물체 회전까지 고려한다고 한다.
	// 충돌 검사나 절두체 잘림 판별에서 쓰인다.
	// 삼차원에서 RECT 두 사선 점을 구한다고 보면 된다.
	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity); // 최소값은 제일 크게 세팅
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity); // 최댓값은 제일 작게 세팅
	
	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
	std::vector<Vertex::Basic32> vertices(vcount);
	for(UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		
		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);
		
		// XMVectorMin, Max 함수는 각 좌표를 min, max 함수에 넣고 돌리는 것이다.
		vMin = XMVectorMin(vMin, P); // 제일 작은 위치에 있는 정점 탐색
		vMax = XMVectorMax(vMax, P); // 제일 큰 위치에 있는 정점 탐색
	}
	
	// 최대, 최소 정점을 중점 한계 표현으로 변환
	XMStoreFloat3(&mSkullBox.Center, 0.5f*(vMin+vMax));
	XMStoreFloat3(&mSkullBox.Extents, 0.5f*(vMax-vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3*tcount;
	std::vector<UINT> indices(mSkullIndexCount);
	for(UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
	}

	fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}

void InstancingAndCullingApp::BuildInstancedBuffer()
{
	// 3차원 공간에 해골을 박스 형태로 그릴 것이기에 인스턴싱도 3차원
	const int n = 5; // 인스턴스 버퍼를 3차원 5x5x5 배열로 생성
	mInstancedData.resize(n*n*n); // 저장은 선형으로 이루어짐
	
	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;
	
	// 지면 격자랑 같은 원리
	float x = -0.5f*width;
	float y = -0.5f*height;
	float z = -0.5f*depth;

	// 증가율
	float dx = width / (n-1);
	float dy = height / (n-1);
	float dz = depth / (n-1);

	for(int k = 0; k < n; ++k)
	{
		for(int i = 0; i < n; ++i)
		{
			for(int j = 0; j < n; ++j)
			{
				// 인스턴스들을 3차원 격자 형태로 배치한다.
				mInstancedData[k*n*n + i*n + j].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x+j*dx, y+i*dy, z+k*dz, 1.0f); // Transelation 행렬, 좌표는 지면 격자마냥 각기 다르다
				
				// 무작위 색상 설정
				mInstancedData[k*n*n + i*n + j].Color.x = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i*n + j].Color.y = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i*n + j].Color.z = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i*n + j].Color.w = 1.0f;
			}
		}
	}
	
	D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC; // 동적으로 설정한다.
	vbd.ByteWidth = sizeof(InstancedData) * mInstancedData.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	
    HR(md3dDevice->CreateBuffer(&vbd, 0, &mInstancedBuffer));
}