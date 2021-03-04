// 언덕을 오르거나 내려가는 경우
// 비정상적으로 빠르게 진행되는 현상을 방지해보는 예제다.
// Update 쪽의 카메라 움직이는 부분을 바꾸었고 Camera.h에 이동 함수를 추가했다.
// 이동 끝 부분 측정하는 부분에서 여기에는
// 이동 끝 부분을 시점이나 횡 이동을 한 후의 x,z 좌표를 가지고 최종적인
// 이동 끝 부분의 x,y,z를 획득한다.

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "Camera.h"
#include "Sky.h"
#include "Terrain.h"

#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dx11d.lib")
#pragma comment (lib,"D3DCompiler.lib")
#pragma comment (lib,"Effects11d.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"winmm.lib")

class TerrainApp : public D3DApp
{
public:
	TerrainApp(HINSTANCE hInstance);
	~TerrainApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	Sky* mSky;
	Terrain mTerrain;

	DirectionalLight mDirLights[3];

	Camera mCam;

	bool mWalkCamMode;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	TerrainApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

TerrainApp::TerrainApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mSky(0), mWalkCamMode(false)
{
	mMainWndCaption = L"Terrain Demo";
	mEnable4xMsaa = false;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, 100.0f);

	mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);
}

TerrainApp::~TerrainApp()
{
	md3dImmediateContext->ClearState();

	SafeDelete(mSky);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool TerrainApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	mSky = new Sky(md3dDevice, L"Textures/grasscube1024.dds", 5000.0f);

	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Textures/terrain.raw";
	tii.LayerMapFilename0 = L"Textures/grass.dds";
	tii.LayerMapFilename1 = L"Textures/darkdirt.dds";
	tii.LayerMapFilename2 = L"Textures/stone.dds";
	tii.LayerMapFilename3 = L"Textures/lightdirt.dds";
	tii.LayerMapFilename4 = L"Textures/snow.dds";
	tii.BlendMapFilename = L"Textures/blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;
	tii.CellSpacing = 0.5f;

	mTerrain.Init(md3dDevice, md3dImmediateContext, tii);
	return true;
}

void TerrainApp::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);
}

void TerrainApp::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
	XMFLOAT3 OldcamPos = mCam.GetPosition();

	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(10.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-10.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCam.Strafe(-10.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCam.Strafe(10.0f * dt);

	//
	// Walk/fly mode
	//
	if (GetAsyncKeyState('2') & 0x8000)
		mWalkCamMode = true;
	if (GetAsyncKeyState('3') & 0x8000)
		mWalkCamMode = false;

	// 기존의 움직이는 방식은 시선 방향으로 카메라를 이동하였다.
	// 이러면 문제가 언덕을 올라가는 경우에 시선을 앞방향을 바라보고 있다면
	// 언덕을 엄청 빠르게 올라가는 현상이 발생한다. 
	// 이유는 x,z 좌표는 실제로 앞방향으로 나아가고 그에 맞춰서 y가 결정되기 때문이다.
	// 이를 해결하려면 시작점에서 이동할 끝점으로의 벡터를 구하고 그 벡터를 따라서
	// 카메라를 이동시키면 된다.
	if (mWalkCamMode)
	{
		XMFLOAT3 camPos = mCam.GetPosition(); // 이동한 방향의 x,z 좌표를 구한다.
		mCam.SetPosition(OldcamPos); // 이동하지 않은 원 상태로 돌린다.
		float NewY = mTerrain.GetHeight(camPos.x, camPos.z);
		float OldY = mTerrain.GetHeight(OldcamPos.x, OldcamPos.z);
		float len = sqrtf(powf(camPos.x - OldcamPos.x, 2) + powf(NewY - OldY, 2) + powf(camPos.z - OldcamPos.z, 2));
		len = !len ? 1 : len;
		// 정규화된 이동할 방향 벡터를 구한다.
		XMFLOAT3 fowardVec = { (camPos.x - OldcamPos.x) / len, (NewY - OldY) / len, (camPos.z - OldcamPos.z) / len };
		mCam.Move(fowardVec, 10.0f * dt); // 구해진 벡터를 방향삼아 이동한다.
		camPos = mCam.GetPosition();
		mCam.SetPosition(camPos.x, mTerrain.GetHeight(camPos.x, camPos.z) + 2.0f, camPos.z);
	}

	mCam.UpdateViewMatrix();
}

void TerrainApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (GetAsyncKeyState('1') & 0x8000)
		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

	mTerrain.Draw(md3dImmediateContext, mCam, mDirLights);

	md3dImmediateContext->RSSetState(0);

	mSky->Draw(md3dImmediateContext, mCam);

	// restore default states, as the SkyFX changes them in the effect file.
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	HR(mSwapChain->Present(0, 0));
}

void TerrainApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void TerrainApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TerrainApp::OnMouseMove(WPARAM btnState, int x, int y)
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
