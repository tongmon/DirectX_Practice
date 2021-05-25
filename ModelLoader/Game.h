//
// Game.h
//

#pragma once

#include "goh.h"
#include "DeviceResources.h"
#include "StepTimer.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "EssEffects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"
#include "RenderStates.h"
#include "ShadowMap.h"
#include "Ssao.h"
#include "TextureMgr.h"
#include "BasicModel.h"
#include "SkinnedModel.h"
#include "ModelLoader.h"
#include "Model.h"

// Effects11.lib, DirectXTK.lib 이 라이브러리는 사전에 Windows Kit에 들어가 있어야 한다.
// 사전에 라이브러리 준비하는 방법은 텍스쳐 파일 설명 참조
#if defined( DEBUG ) || defined( _DEBUG )
#pragma comment (lib,"Effects11d.lib")
#pragma comment (lib,"DirectXTKd.lib")
#else
#pragma comment (lib,"Effects11.lib")
#pragma comment (lib,"DirectXTK.lib")
#endif

// 라이브러리 참조
#pragma comment (lib,"assimp-vc142-mtd.lib")

// D3D11 device 생성 및 게임 루프 제공
// final -> 가상 함수 사용은 여기서 끝내겠다. 더 이상의 가상 함수는 없다는 것을 명시
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false); // noexcept -> 오류 던지지 않는다는 보장을 함으로써 좀 더 빠른 속도 획득
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // 기본적인 Game 루프
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // WinApi 메시지 담당
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;
    HRESULT	CompileShaderFromFile(LPCWSTR pFileName, const D3D_SHADER_MACRO* pDefines, LPCSTR pEntryPoint, LPCSTR pShaderModel, ID3DBlob** ppBytecodeBlob);

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // 장치 자원들 집합
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // 타이머
    DX::StepTimer                           m_timer;

    // 예전 마우스 좌표
    POINT                                   mLastMousePos;

private:
    void DrawSceneToSsaoNormalDepthMap();
    void DrawSceneToShadowMap();
    void DrawScreenQuad(ID3D11ShaderResourceView* srv);
    void BuildShadowTransform();
    void BuildShapeGeometryBuffers();
    void BuildSkullGeometryBuffers();
    void BuildScreenQuadGeometryBuffers();

private:
    TextureMgr mTexMgr;

    Sky* mSky;

    Model* mTestModels[4];
    // 테스트 모델
    Model* mTest;
    ModelInst mTestInst_1;

    // 키 입력
    bool mKeyInput[50];

    ID3D11Buffer* mShapesVB;
    ID3D11Buffer* mShapesIB;

    ID3D11Buffer* mSkullVB;
    ID3D11Buffer* mSkullIB;

    ID3D11Buffer* mSkySphereVB;
    ID3D11Buffer* mSkySphereIB;

    ID3D11Buffer* mScreenQuadVB;
    ID3D11Buffer* mScreenQuadIB;

    ID3D11ShaderResourceView* mStoneTexSRV;
    ID3D11ShaderResourceView* mBrickTexSRV;

    ID3D11ShaderResourceView* mStoneNormalTexSRV;
    ID3D11ShaderResourceView* mBrickNormalTexSRV;

    // 장면 전체를 덮는 구, 장면에 대한 절두체를 계산할 때 이용
    DirectX::BoundingSphere mSceneBounds;

    // 그림자 맵에 대한 크기, 빛 방향 등의 정보
    static const int SMapSize = 2048;
    ShadowMap* mSmap;
    XMFLOAT4X4 mLightView;
    XMFLOAT4X4 mLightProj;
    XMFLOAT4X4 mShadowTransform;

    Ssao* mSsao; // AO에 대한 포인터

    float mLightRotationAngle;
    XMFLOAT3 mOriginalLightDir[3];
    DirectionalLight mDirLights[3];
    Material mGridMat;
    Material mBoxMat;
    Material mCylinderMat;
    Material mSphereMat;
    Material mSkullMat;

    // 물체에 대한 변환 행렬
    XMFLOAT4X4 mSphereWorld[10];
    XMFLOAT4X4 mCylWorld[10];
    XMFLOAT4X4 mBoxWorld;
    XMFLOAT4X4 mGridWorld;
    XMFLOAT4X4 mSkullWorld;

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

    UINT mSkullIndexCount;

    Camera mCam; // 카메라
    
    // 카메라로부터 플레이어와의 거리
    float mCamDist;
    XMFLOAT3 mCamCenter;
};
