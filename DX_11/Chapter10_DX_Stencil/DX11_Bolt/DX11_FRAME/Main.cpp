// 전기 예제
// 전기 예제를 하면서 많이 고생을 했다....
// 일단 하면서 의문이 든 것이 크게 3가지가 있다.
// 1. 전기 텍스쳐 렌더링 테크 패스에 안개를 추가하면 문제가 되는 것
// 2. 가산 혼합하면 왜 검은색 부분이 투명처럼 보이나
// 3. 깊이 안쓰고 전기 원기둥을 그리는데 어떻게 카메라에 더 가까운 지면에 원기둥이 가려지나
// 1번은 안개를 전기 원기둥에 추가하면 투명했던 부분이 부자연스러워지는데 이는 생각해보니 당연하다.
// 먼저 지면, 물을 그리고 패스를 돌리면서 안개를 씌운 배경에다가 다시 안개를 또 씌우니까
// 투명 부분에 더 짙은 안개가 끼게 된다.
// 2번, 3번은 DrawScene 함수에다가 내가 찾은 이유를 적어놓았다.
// 1,2,3번에 대한 궁금증이 모두 풀리게된 예제였다....

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "Waves.h"

#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dx11d.lib")
#pragma comment (lib,"D3DCompiler.lib")
#pragma comment (lib,"Effects11d.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"winmm.lib")

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class BlendApp : public D3DApp
{
public:
	BlendApp(HINSTANCE hInstance);
	~BlendApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildBoltGeometryBuffers(float radius, float length);

private:
	// 지면 정점, 인덱스
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;

	// 물 정점, 인덱스
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	// 전기 원기둥 정점, 인덱스
	ID3D11Buffer* mBoltCylinderVB;
	ID3D11Buffer* mBoltCylinderIB;

	// 텍스쳐 리소스 할당
	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWavesMapSRV;
	ID3D11ShaderResourceView* mBoltMapSRV[60];

	Waves mWaves; // 파도 클래스(물 생성)

	DirectionalLight mDirLights[3]; // 방향광 3개

	// 각종 물체 특성, 전역색, 물체색, 반영광 받는 색
	Material mLandMat;
	Material mWavesMat;
	Material mBoltMat;

	// 텍스쳐 변환 행렬
	XMFLOAT4X4 mGrassTexTransform;
	XMFLOAT4X4 mWaterTexTransform;
	XMFLOAT4X4 mBoltTexTransform;

	// 물체 변환 행렬
	XMFLOAT4X4 mLandWorld;
	XMFLOAT4X4 mWavesWorld;
	XMFLOAT4X4 mBoltWorld;

	// 시점, 투영 행렬
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	// 지면, 전기 원기둥의 인덱스 카운트
	UINT mLandIndexCount;
	UINT mBoltIndexCount;

	UINT mBoltFrameIndex; // 전기 텍스쳐 60개 돌리는 인덱스

	// 물 텍스쳐 위치 지정
	XMFLOAT2 mWaterTexOffset;

	// 렌더링 옵션
	RenderOptions mRenderOptions;

	// 카메라 위치
	XMFLOAT3 mEyePosW;

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

	BlendApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

BlendApp::BlendApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mLandVB(0), mLandIB(0), mWavesVB(0), mWavesIB(0), mBoltCylinderVB(0), mBoltCylinderIB(0), mGrassMapSRV(0), mWavesMapSRV(0),
	mWaterTexOffset(0.0f, 0.0f), mEyePosW(0.0f, 0.0f, 0.0f), mLandIndexCount(0), mBoltIndexCount(0), mBoltFrameIndex(0),
	mRenderOptions(RenderOptions::TexturesAndFog), mTheta(1.65f * MathHelper::Pi), mPhi(0.3f * MathHelper::Pi), mRadius(60.0f)
{
	mMainWndCaption = L"Bolt Demo";
	mEnable4xMsaa = false;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	// 행렬 초기화
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	// 전기 원기둥 변환 행렬
	XMMATRIX boltScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX boltOffset = XMMatrixTranslation(8.0f, 5.0f, -15.0f);
	XMStoreFloat4x4(&mBoltWorld, boltScale * boltOffset);

	// 풀 텍스쳐 격자 확장
	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

	// 빛 속성 설정
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

	mLandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mWavesMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	mBoltMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoltMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoltMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}

BlendApp::~BlendApp()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mBoltCylinderVB);
	ReleaseCOM(mBoltCylinderIB);
	ReleaseCOM(mGrassMapSRV);
	ReleaseCOM(mWavesMapSRV);
	for (int i = 0; i < 60; ++i)
		ReleaseCOM(mBoltMapSRV[i]);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool BlendApp::Init()
{
	if (!D3DApp::Init())
		return false;

	mWaves.Init(160, 160, 1.0f, 0.03f, 5.0f, 0.3f);

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"Textures/grass.dds", 0, 0, &mGrassMapSRV, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"Textures/water2.dds", 0, 0, &mWavesMapSRV, 0));

	for (int i = 0; i < 60; ++i)
	{
		std::wstring filename = L"Textures/Bolt";

		if (i + 1 <= 9)
			filename += L"0";

		if (i + 1 <= 99)
			filename += L"0";

		std::wstringstream frameNum;
		frameNum << i + 1;
		filename += frameNum.str();
		filename += L".bmp";

		HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
			filename.c_str(), 0, 0, &mBoltMapSRV[i], 0));
	}

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildBoltGeometryBuffers(12.0f, 15.0f);

	return true;
}

void BlendApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BlendApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	//
	// Every quarter second, generate a random wave.
	//
	static float t_base = 0.0f;
	if ((mTimer.TotalTime() - t_base) >= 0.1f)
	{
		t_base += 0.1f;

		DWORD i = 5 + rand() % (mWaves.RowCount() - 10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);

		float r = MathHelper::RandF(0.5f, 1.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	//
	// Update the wave vertex buffer with the new solution.
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(md3dImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos = mWaves[i];
		v[i].Normal = mWaves.Normal(i);

		// Derive tex-coords in [0,1] from position.
		v[i].Tex.x = 0.5f + mWaves[i].x / mWaves.Width();
		v[i].Tex.y = 0.5f - mWaves[i].z / mWaves.Depth();
	}

	md3dImmediateContext->Unmap(mWavesVB, 0);

	//
	// Animate water texture coordinates.
	//

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	mWaterTexOffset.y += 0.05f * dt;
	mWaterTexOffset.x += 0.1f * dt;
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&mWaterTexTransform, wavesScale * wavesOffset);

	//
	// Animate bolt texture.
	//

	// 전기 원기둥 텍스쳐 변환 행렬 업데이트
	XMMATRIX boltScale = XMMatrixScaling(3.0f, 1.5f, 1.0f);
	XMMATRIX boltTranslation = XMMatrixTranslation(mTimer.TotalTime() * 0.02f, 0.0f, 0.0f);
	XMStoreFloat4x4(&mBoltTexTransform, boltScale * boltTranslation);


	//
	// 전기 텍스쳐 인덱스 수정
	//

	static float t = 0.0f;
	t += dt;

	if (t >= 0.033333f)
	{
		mBoltFrameIndex++;
		t = 0.0f;
		if (mBoltFrameIndex == 60)
			mBoltFrameIndex = 0;
	}

	//
	// Switch the render mode based in key input.
	//
	if (GetAsyncKeyState('1') & 0x8000)
		mRenderOptions = RenderOptions::Lighting;

	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOptions = RenderOptions::Textures;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOptions = RenderOptions::TexturesAndFog;
}

void BlendApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);
	Effects::BasicFX->SetFogColor(Colors::Silver);
	Effects::BasicFX->SetFogStart(15.0f);
	Effects::BasicFX->SetFogRange(175.0f);

	ID3DX11EffectTechnique* boltTech = NULL;
	ID3DX11EffectTechnique* landAndWavesTech = NULL;

	// 여기서 boltTech는 Light3TexAlphaClipTech 상태가 아닌
	// Light0TexTech 상태여야 한다. 일단 Bolt텍스쳐 파일 자체가
	// 알파값이 설정된 파일이 아니라 버려지는 픽셀이 없다.
	switch (mRenderOptions)
	{
	case RenderOptions::Lighting:
		boltTech = Effects::BasicFX->Light0TexTech;
		landAndWavesTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		boltTech = Effects::BasicFX->Light0TexTech;
		landAndWavesTech = Effects::BasicFX->Light3TexTech;
		break;
	case RenderOptions::TexturesAndFog:
		boltTech = Effects::BasicFX->Light0TexTech;
		landAndWavesTech = Effects::BasicFX->Light3TexFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;

	//
	// Draw the hills and water with texture and fog (no alpha clipping needed).
	//

	landAndWavesTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		//
		// Draw the hills.
		//
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mGrassTexTransform));
		Effects::BasicFX->SetMaterial(mLandMat);
		Effects::BasicFX->SetDiffuseMap(mGrassMapSRV);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

		//
		// Draw the waves.
		//
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		world = XMLoadFloat4x4(&mWavesWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mWaterTexTransform));
		Effects::BasicFX->SetMaterial(mWavesMat);
		Effects::BasicFX->SetDiffuseMap(mWavesMapSRV);

		md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		landAndWavesTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

		// Restore default blend state
		md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
	}

	//
	// 깊이 버퍼가 없이 가산 혼합을 사용하여 전기 원기둥 렌더링
	// 어떤 물체 내에서도 거리가 앞이고, 거리가 뒤이고 이런 앞 뒤가 존재한다.
	// 바로 이 전기 원기둥이 그런데 이 물체를 그릴 때 만약 깊이를 사용하여
	// 렌더링을 하면 원기둥 앞면의 전기 텍스쳐가 뒷면의 전기 텍스쳐를 가려버리는
	// 이러한 효과가 나타난다.
	// 이러한 효과를 없애려면 깊이 버퍼에 깊이값 쓰기를 하지 않고 그냥
	// 물건 자체의 앞뒤가 없는 것 처럼 그리면 된다.
	// 물건 자체의 깊이가 없는 것이지 깊이 판정은 존재하여 지면에 가려질 곳을 가려진다.
	// 깊이 판정이 존재한다는 것은 이 물체를 그릴 때 이 물체에 대한 좌표를 기반으로 가릴지 말지 결정은 한다는 것이다.
	// 이 물체가 지면 보다 더 앞에 있다면 그려지긴 하겠지만 깊이 버퍼를 갱신하지는 않는다.
	// 혼합 기법으로 가산 혼합을 사용했는데 원래 텍스쳐에서 검하던 색이 투명색과 같이 보이는 것은
	// (0,0,0)[전기 텍스쳐 검은색 부분] * (1,1,1) + (어떤 배경 3원색) * (1,1,1)을 진행하면 배경색만 보인다.
	// 물론 이는 전기 텍스쳐의 전기 부분도 마찬가지로 이루어진다.
	// 

	boltTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoltCylinderVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoltCylinderIB, DXGI_FORMAT_R16_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mBoltWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mBoltTexTransform));
		Effects::BasicFX->SetMaterial(mBoltMat);
		Effects::BasicFX->SetDiffuseMap(mBoltMapSRV[mBoltFrameIndex]);

		md3dImmediateContext->RSSetState(RenderStates::NoCullRS);
		md3dImmediateContext->OMSetBlendState(RenderStates::AdditiveBS, blendFactor, 0xffffffff);
		//md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		md3dImmediateContext->OMSetDepthStencilState(RenderStates::DepthWriteOffDSS, 0);
		boltTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoltIndexCount, 0, 0);

		// Restore default render state.
		md3dImmediateContext->RSSetState(0);
		md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		md3dImmediateContext->OMSetDepthStencilState(0, 0);
	}


	HR(mSwapChain->Present(0, 0));
}

void BlendApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BlendApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BlendApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.1f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 20.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

float BlendApp::GetHillHeight(float x, float z)const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 BlendApp::GetHillNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void BlendApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//

	std::vector<Vertex::Basic32> vertices(grid.Vertices.size());
	for (UINT i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHillHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex = grid.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mLandIB));
}

void BlendApp::BuildWaveGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(md3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));


	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.

	std::vector<UINT> indices(3 * mWaves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));
}

void BlendApp::BuildBoltGeometryBuffers(float radius, float length)
{
	// Make a cylinder with one stack and no end-caps.

	const UINT numSlices = 32;

	const UINT numCircleVerts = 33;
	const UINT vertexCount = 2 * numCircleVerts;

	float dTheta = 2.0f * XM_PI / numSlices;

	Vertex::Basic32 vertices[vertexCount];

	for (UINT i = 0; i < numCircleVerts; ++i)
	{
		float x = radius * cosf(i * dTheta);
		float z = radius * sinf(i * dTheta);

		vertices[i].Pos = XMFLOAT3(x, 0.0f, z);
		vertices[i].Normal = XMFLOAT3(x, 0.0f, z);
		vertices[i].Tex = XMFLOAT2(i * dTheta / (2.0f * XM_PI), 0.95f);

		vertices[i + numCircleVerts].Pos = XMFLOAT3(x, length, z);
		vertices[i + numCircleVerts].Normal = XMFLOAT3(x, 0.0f, z);
		vertices[i + numCircleVerts].Tex = XMFLOAT2(i * dTheta / (2.0f * XM_PI), 0.05f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoltCylinderVB));

	mBoltIndexCount = numSlices * 6;
	USHORT indices[numSlices * 6]; // 6 indices per quad

	for (UINT i = 0; i < numSlices; ++i)
	{
		indices[i * 6 + 0] = i;
		indices[i * 6 + 1] = numCircleVerts + i;
		indices[i * 6 + 2] = numCircleVerts + i + 1;

		indices[i * 6 + 3] = i;
		indices[i * 6 + 4] = numCircleVerts + i + 1;
		indices[i * 6 + 5] = i + 1;
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * mBoltIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoltCylinderIB));
}
