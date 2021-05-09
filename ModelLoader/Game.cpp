//
// Game.cpp
//

#include "goh.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace PackedVector;

using Microsoft::WRL::ComPtr;

struct ConstantBuffer {
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
};

Game::Game() noexcept(false) : mSky(0),
mShapesVB(0), mShapesIB(0), mSkullVB(0), mSkullIB(0), mScreenQuadVB(0), mScreenQuadIB(0),
mStoneTexSRV(0), mBrickTexSRV(0),
mStoneNormalTexSRV(0), mBrickNormalTexSRV(0),
mSkullIndexCount(0), mSmap(0), mSsao(0),
mLightRotationAngle(0.0f)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
    
    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    // 장면에 대한 크기는 미리 구해서 알고 있는 상황이기에 이렇게 미리 정함
    // 원래는 장면 전체에 대한 정점들 Min, Max 거리 구해서 반지름 획득해야됨
    mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    mSceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mGridWorld, XMMatrixScaling(3, 3, 3));

    XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
    XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
    XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

    XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

    for (int i = 0; i < 5; ++i)
    {
        XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

        XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
    }

    mDirLights[0].Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(1.0f, 0.9f, 0.9f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    // Shadow acne gets worse as we increase the slope of the polygon (from the
    // perspective of the light).
    //mDirLights[0].Direction = XMFLOAT3(5.0f/sqrtf(50.0f), -5.0f/sqrtf(50.0f), 0.0f);
    //mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(125.0f), -5.0f/sqrtf(125.0f), 0.0f);
    //mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(116.0f), -4.0f/sqrtf(116.0f), 0.0f);
    //mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(109.0f), -3.0f/sqrtf(109.0f), 0.0f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);

    mOriginalLightDir[0] = mDirLights[0].Direction;
    mOriginalLightDir[1] = mDirLights[1].Direction;
    mOriginalLightDir[2] = mDirLights[2].Direction;

    mGridMat.Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    mGridMat.Diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    mGridMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
    mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mCylinderMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mCylinderMat.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mCylinderMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
    mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mSphereMat.Ambient = XMFLOAT4(0.3f, 0.4f, 0.5f, 1.0f);
    mSphereMat.Diffuse = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
    mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
    mSphereMat.Reflect = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

    mBoxMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mSkullMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mSkullMat.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 16.0f);
    mSkullMat.Reflect = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
}

Game::~Game()
{
    SafeDelete(mSky);
    SafeDelete(mSmap);
    SafeDelete(mSsao);
    SafeDelete(mTest);

    for (int i = 0; i < 4; i++)
    {
        SafeDelete(mTestModels[i]);
    }

    ReleaseCOM(mShapesVB);
    ReleaseCOM(mShapesIB);
    ReleaseCOM(mSkullVB);
    ReleaseCOM(mSkullIB);
    ReleaseCOM(mScreenQuadVB);
    ReleaseCOM(mScreenQuadIB);
    
    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

// 시작시에 Direct3D 자원 초기화
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    Effects::InitAll(device);
    InputLayouts::InitAll(device);
    RenderStates::InitAll(device);
    
    /*
    // 모델 로더 테스트
    ModelLoader::LoadModel(m_deviceResources->GetWindow(), device, context,
        "Models\\Paladin_sh_sw\\paladin_prop_j_nordstrom.fbx", "Models\\LoadedModels\\Paladin_sh_sw", "Models\\LoadedModels\\Paladin_sh_sw\\Texture");
    ModelLoader::LoadAnimation("Models\\Paladin_sh_sw\\sword and shield death.fbx");
    ModelLoader::LoadAnimation("Models\\Paladin_sh_sw\\sword and shield jump (2).fbx");
    ModelLoader::LoadAnimation("Models\\Paladin_sh_sw\\sword and shield walk.fbx");
    ModelLoader::LoadAnimation("Models\\Paladin_sh_sw\\sword and shield slash (2).fbx");
    ModelLoader::LoadAnimation("Models\\Paladin_sh_sw\\falling to roll.fbx");
    */

    /*
    ModelLoader::LoadModel(m_deviceResources->GetWindow(), device, context,
        "Models\\Biking\\Ch19_nonPBR.fbx", "Models\\LoadedModels\\Biking", "Models\\LoadedModels\\Biking\\Texture");
    ModelLoader::LoadAnimation("Models\\Biking\\idle.fbx");
    ModelLoader::LoadAnimation("Models\\Biking\\running.fbx");
    ModelLoader::LoadAnimation("Models\\Biking\\walking.fbx");

    ModelLoader::LoadModel(m_deviceResources->GetWindow(), device, context,
        "Models\\Laygo\\maw_j_laygo.fbx", "Models\\LoadedModels\\Laygo", "Models\\LoadedModels\\Laygo\\Texture");
    ModelLoader::LoadAnimation("Models\\Laygo\\idle.fbx");
    ModelLoader::LoadAnimation("Models\\Laygo\\running.fbx");
    ModelLoader::LoadAnimation("Models\\Laygo\\walking.fbx");

    ModelLoader::LoadModel(m_deviceResources->GetWindow(), device, context,
        "Models\\Kachujin\\kachujin_g_rosales.fbx", "Models\\LoadedModels\\Kachujin", "Models\\LoadedModels\\Kachujin\\Texture");
    ModelLoader::LoadAnimation("Models\\Kachujin\\idle.fbx");
    ModelLoader::LoadAnimation("Models\\Kachujin\\running.fbx");
    ModelLoader::LoadAnimation("Models\\Kachujin\\walking.fbx");

    ModelLoader::LoadModel(m_deviceResources->GetWindow(), device, context,
        "Models\\Ninja\\ninja.fbx", "Models\\LoadedModels\\Ninja", "Models\\LoadedModels\\Ninja\\Texture");
    ModelLoader::LoadAnimation("Models\\Ninja\\idle.fbx");
    ModelLoader::LoadAnimation("Models\\Ninja\\running.fbx");
    ModelLoader::LoadAnimation("Models\\Ninja\\walking.fbx");
    */

    /*
    ModelLoader::LoadModel(m_deviceResources->GetWindow(), device, context,
        "Models\\Vampi\\vampire_a_lusth.fbx", "Models\\LoadedModels\\Vampi", "Models\\LoadedModels\\Vampi\\Texture");
    ModelLoader::LoadAnimation("Models\\Vampi\\right turn.fbx");
    */

    mTexMgr.Init(device);

    mSky = new Sky(device, L"Textures/desertcube1024.dds", 5000.0f);
    mSmap = new ShadowMap(device, SMapSize, SMapSize);

    mCam.SetLens(0.25f * XM_PI, m_deviceResources->GetAspectRatio(), 1.0f, 1000.0f);
    mSsao = new Ssao(device, context, m_deviceResources->GetClientWidth(), m_deviceResources->GetClientHeight(), mCam.GetFovY(), mCam.GetFarZ());

    // 각 이미지에 대한 쉐이더 리소스뷰 획득
    mStoneTexSRV = mTexMgr.CreateTexture(L"Textures/floor.dds");
    mBrickTexSRV = mTexMgr.CreateTexture(L"Textures/bricks.dds");
    mStoneNormalTexSRV = mTexMgr.CreateTexture(L"Textures/floor_nmap.dds");
    mBrickNormalTexSRV = mTexMgr.CreateTexture(L"Textures/bricks_nmap.dds");

    BuildShapeGeometryBuffers();
    BuildSkullGeometryBuffers();
    BuildScreenQuadGeometryBuffers();

    /*
    // 테스트 모델
    mTest = new Model(device, mTexMgr, "Models\\LoadedModels\\Paladin_sh_sw\\paladin_prop_j_nordstrom.model");
    mTest->AddAnimation("Models\\LoadedModels\\Paladin_sh_sw\\sword and shield walk_1.anim", "walk");
    mTest->AddAnimation("Models\\LoadedModels\\Paladin_sh_sw\\sword and shield jump (2)_1.anim", "jump");
    mTest->AddAnimation("Models\\LoadedModels\\Paladin_sh_sw\\sword and shield slash (2)_1.anim", "attack");
    mTestInst_1.mModel = mTest;
    mTestInst_2.mModel = mTest;
    mTestInst_1.mClipName = "walk";
    mTestInst_2.mClipName = "jump";
    mTestInst_1.mFinalTransforms.resize(mTest->mBoneOffset.size());
    mTestInst_2.mFinalTransforms.resize(mTest->mBoneOffset.size());
    */

    mTest = new Model(device, mTexMgr, "Models\\LoadedModels\\Vampi\\vampire_a_lusth.model");
    mTest->AddAnimation("Models\\LoadedModels\\Vampi\\walking_1.anim", "walk");
    mTest->AddAnimation("Models\\LoadedModels\\Vampi\\running_1.anim", "run");
    mTest->AddAnimation("Models\\LoadedModels\\Vampi\\idle_1.anim", "idle");
    mTest->AddAnimation("Models\\LoadedModels\\Vampi\\left turn_1.anim", "l_turn");
    mTest->AddAnimation("Models\\LoadedModels\\Vampi\\right turn_1.anim", "r_turn");

    mTestModels[0] = new Model(device, mTexMgr, "Models\\LoadedModels\\Biking\\Ch19_nonPBR.model");
    mTestModels[0]->AddAnimation("Models\\LoadedModels\\Biking\\walking_1.anim", "walk");
    mTestModels[0]->AddAnimation("Models\\LoadedModels\\Biking\\running_1.anim", "run");
    mTestModels[0]->AddAnimation("Models\\LoadedModels\\Biking\\idle_1.anim", "idle");

    mTestModels[1] = new Model(device, mTexMgr, "Models\\LoadedModels\\Laygo\\maw_j_laygo.model");
    mTestModels[1]->AddAnimation("Models\\LoadedModels\\Laygo\\walking_1.anim", "walk");
    mTestModels[1]->AddAnimation("Models\\LoadedModels\\Laygo\\running_1.anim", "run");
    mTestModels[1]->AddAnimation("Models\\LoadedModels\\Laygo\\idle_1.anim", "idle");

    mTestModels[2] = new Model(device, mTexMgr, "Models\\LoadedModels\\Kachujin\\kachujin_g_rosales.model");
    mTestModels[2]->AddAnimation("Models\\LoadedModels\\Kachujin\\walking_1.anim", "walk");
    mTestModels[2]->AddAnimation("Models\\LoadedModels\\Kachujin\\running_1.anim", "run");
    mTestModels[2]->AddAnimation("Models\\LoadedModels\\Kachujin\\idle_1.anim", "idle");

    mTestModels[3] = new Model(device, mTexMgr, "Models\\LoadedModels\\Ninja\\ninja.model");
    mTestModels[3]->AddAnimation("Models\\LoadedModels\\Ninja\\walking_1.anim", "walk");
    mTestModels[3]->AddAnimation("Models\\LoadedModels\\Ninja\\running_1.anim", "run");
    mTestModels[3]->AddAnimation("Models\\LoadedModels\\Ninja\\idle_1.anim", "idle");
    
    mTestInst_1.mScale = 0.03f;
    mTestInst_1.mAngle = 0;
    mTestInst_1.mX = -2.f;
    mTestInst_1.mY = 0.f;
    mTestInst_1.mZ = -7.f;
    
    XMFLOAT3 S = { mTestInst_1.mScale,mTestInst_1.mScale,mTestInst_1.mScale };
    XMFLOAT3 T = { mTestInst_1.mX, mTestInst_1.mY, mTestInst_1.mZ };
    XMMATRIX Result = XMMatrixAffineTransformation(XMLoadFloat3(&S), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
            XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), mTestInst_1.mAngle), XMLoadFloat3(&T));
    XMStoreFloat4x4(&mTestInst_1.mWorld, Result);

    mTestInst_1.mState = 5;
    mTestInst_1.mAngle = 0.f;
    mTestInst_1.mModel = mTest;
    mTestInst_1.mClipName = "idle";
    mTestInst_1.mFinalTransforms.resize(mTest->mBoneOffset.size());

    mCamDist = 13;
    mCamCenter = { mTestInst_1.mX, mTestInst_1.mY + 5, mTestInst_1.mZ };
    XMFLOAT3 L = mCam.GetLook();
    mCam.SetPosition(mCamCenter.x - L.x * mCamDist, mCamCenter.y - L.y * mCamDist, mCamCenter.z - L.z * mCamDist);

    // TODO: 타이머 설정을 원하면 바꿀 수 있다.
    // 밑의 예시는 60 프레임 고정으로 설정하는 방법이다.
    /*
    m_timer.SetFixedTimeStep(true); // 고정 프레임 사용
    m_timer.SetTargetElapsedSeconds(1.0 / 60); // 60 프레임 고정
    */
}

HRESULT	Game::CompileShaderFromFile(LPCWSTR pFileName, const D3D_SHADER_MACRO* pDefines, LPCSTR pEntryPoint, LPCSTR pShaderModel, ID3DBlob** ppBytecodeBlob)
{
    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob = nullptr;

    HRESULT result = D3DCompileFromFile(pFileName, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, pEntryPoint, pShaderModel, compileFlags, 0, ppBytecodeBlob, &pErrorBlob);
    if (FAILED(result))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((LPCSTR)pErrorBlob->GetBufferPointer());
    }

    if (pErrorBlob != nullptr)
        pErrorBlob->Release();

    return result;
}

void Game::DrawSceneToSsaoNormalDepthMap()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    
    XMMATRIX view = mCam.View();
    XMMATRIX proj = mCam.Proj();
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    ID3DX11EffectTechnique* tech = Effects::SsaoNormalDepthFX->NormalDepthTech;
    ID3DX11EffectTechnique* animatedTech = Effects::SsaoNormalDepthFX->NormalDepthSkinnedTech;

    XMMATRIX world;
    XMMATRIX worldInvTranspose;
    XMMATRIX worldView;
    XMMATRIX worldInvTransposeView;
    XMMATRIX worldViewProj;

    //
    // Draw the grid, cylinders, spheres and box.

    UINT stride = sizeof(Vertex::PosNormalTexTan);
    UINT offset = 0;

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(InputLayouts::PosNormalTexTan);
    context->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
    context->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

    if (GetAsyncKeyState('1') & 0x8000)
        context->RSSetState(RenderStates::WireframeRS);

    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // Draw the grid.
        world = XMLoadFloat4x4(&mGridWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldView = world * view;
        worldInvTransposeView = worldInvTranspose * view;
        worldViewProj = world * view * proj;

        Effects::SsaoNormalDepthFX->SetWorldView(worldView);
        Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
        Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
        Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));

        tech->GetPassByIndex(p)->Apply(0, context);
        context->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);
    }

    //
    // Draw the animated characters.
    //
    context->RSSetState(RenderStates::NoCullRS);

    if (GetAsyncKeyState('1') & 0x8000)
        context->RSSetState(RenderStates::WireframeRS);

    context->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

    animatedTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        world = XMLoadFloat4x4(&mTestInst_1.mWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldView = world * view;
        worldInvTransposeView = worldInvTranspose * view;
        worldViewProj = world * view * proj;

        Effects::SsaoNormalDepthFX->SetWorldView(worldView);
        Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
        Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
        Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());
        Effects::SsaoNormalDepthFX->SetBoneTransforms(
            &mTestInst_1.mFinalTransforms[0],
            mTestInst_1.mFinalTransforms.size());

        animatedTech->GetPassByIndex(p)->Apply(0, context);

        for (UINT subset = 0; subset < mTestInst_1.mModel->mSubsetCount; ++subset)
        {
            mTestInst_1.mModel->mMeshes.Draw(context, subset);
        }
    }

    context->RSSetState(0);
}

void Game::DrawSceneToShadowMap()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    
    XMMATRIX view = XMLoadFloat4x4(&mLightView);
    XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    
    Effects::BuildShadowMapFX->SetEyePosW(mCam.GetPosition());
    Effects::BuildShadowMapFX->SetViewProj(viewProj);

    // These properties could be set per object if needed.
    Effects::BuildShadowMapFX->SetHeightScale(0.07f);
    Effects::BuildShadowMapFX->SetMaxTessDistance(1.0f);
    Effects::BuildShadowMapFX->SetMinTessDistance(25.0f);
    Effects::BuildShadowMapFX->SetMinTessFactor(1.0f);
    Effects::BuildShadowMapFX->SetMaxTessFactor(5.0f);

    ID3DX11EffectTechnique* smapTech = Effects::BuildShadowMapFX->BuildShadowMapTech;
    ID3DX11EffectTechnique* animatedSmapTech = Effects::BuildShadowMapFX->BuildShadowMapSkinnedTech;

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    XMMATRIX world;
    XMMATRIX worldInvTranspose;
    XMMATRIX worldViewProj;

    //
    // Draw the grid, cylinders, spheres, and box.
    // 

    UINT stride = sizeof(Vertex::PosNormalTexTan);
    UINT offset = 0;

    context->IASetInputLayout(InputLayouts::PosNormalTexTan);
    context->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
    context->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

    if (GetAsyncKeyState('1') & 0x8000)
        context->RSSetState(RenderStates::WireframeRS);

    D3DX11_TECHNIQUE_DESC techDesc;
    smapTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // Draw the grid.
        world = XMLoadFloat4x4(&mGridWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::BuildShadowMapFX->SetWorld(world);
        Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
        Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));

        smapTech->GetPassByIndex(p)->Apply(0, context);
        context->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);
    }   

    //
    // Draw the animated characters.
    //
    context->RSSetState(RenderStates::NoCullRS);

    if (GetAsyncKeyState('1') & 0x8000)
        context->RSSetState(RenderStates::WireframeRS);

    context->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

    animatedSmapTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        world = XMLoadFloat4x4(&mTestInst_1.mWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::BuildShadowMapFX->SetWorld(world);
        Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
        Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());
        Effects::BuildShadowMapFX->SetBoneTransforms(
            &mTestInst_1.mFinalTransforms[0],
            mTestInst_1.mFinalTransforms.size());


        animatedSmapTech->GetPassByIndex(p)->Apply(0, context);

        for (UINT subset = 0; subset < mTestInst_1.mModel->mSubsetCount; ++subset)
        {
            mTestInst_1.mModel->mMeshes.Draw(context, subset);
        }
    }

    context->RSSetState(0);
}

void Game::DrawScreenQuad(ID3D11ShaderResourceView* srv)
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    
    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

    context->IASetInputLayout(InputLayouts::Basic32);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
    context->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);

    // Scale and shift quad to lower-right corner.
    XMMATRIX world(
        0.25f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.25f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.75f, -0.75f, 0.0f, 1.0f);

    ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewRedTech;
    D3DX11_TECHNIQUE_DESC techDesc;

    tech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        Effects::DebugTexFX->SetWorldViewProj(world);
        Effects::DebugTexFX->SetTexture(srv);

        tech->GetPassByIndex(p)->Apply(0, context);
        context->DrawIndexed(6, 0, 0);
    }
}

void Game::BuildShadowTransform()
{
    // 하나의 주 광원만 그림자가 드리우게 설정했다.
    XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
    XMVECTOR lightPos = -2.0f * mSceneBounds.Radius * lightDir;
    XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

    // Transform bounding sphere to light space.
    XMFLOAT3 sphereCenterLS;
    XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

    // Ortho frustum in light space encloses scene.
    float l = sphereCenterLS.x - mSceneBounds.Radius;
    float b = sphereCenterLS.y - mSceneBounds.Radius;
    float n = sphereCenterLS.z - mSceneBounds.Radius;
    float r = sphereCenterLS.x + mSceneBounds.Radius;
    float t = sphereCenterLS.y + mSceneBounds.Radius;
    float f = sphereCenterLS.z + mSceneBounds.Radius;
    XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    XMMATRIX S = V * P * T;

    XMStoreFloat4x4(&mLightView, V);
    XMStoreFloat4x4(&mLightProj, P);
    XMStoreFloat4x4(&mShadowTransform, S);
}

void Game::BuildShapeGeometryBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData grid;
    GeometryGenerator::MeshData sphere;
    GeometryGenerator::MeshData cylinder;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
    geoGen.CreateGrid(20.0f, 30.0f, 50, 40, grid);
    geoGen.CreateSphere(0.5f, 20, 20, sphere);
    geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, cylinder);

    // 도형들 정점에 대한 시작점 설정
    mBoxVertexOffset = 0;
    mGridVertexOffset = box.Vertices.size();
    mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
    mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

    // 각 도형에 대한 인덱스 크기 설정
    mBoxIndexCount = box.Indices.size();
    mGridIndexCount = grid.Indices.size();
    mSphereIndexCount = sphere.Indices.size();
    mCylinderIndexCount = cylinder.Indices.size();

    // 각 도형의 인덱스 시작점 설정
    mBoxIndexOffset = 0;
    mGridIndexOffset = mBoxIndexCount;
    mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
    mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

    UINT totalVertexCount =
        box.Vertices.size() +
        grid.Vertices.size() +
        sphere.Vertices.size() +
        cylinder.Vertices.size();

    UINT totalIndexCount =
        mBoxIndexCount +
        mGridIndexCount +
        mSphereIndexCount +
        mCylinderIndexCount;

    //
    // 인덱스, 정점을 모두 하나의 저장소에 몰아 저장한다. 캐시 힛을 줄여서 속도가 상승한다.
    //

    std::vector<Vertex::PosNormalTexTan> vertices(totalVertexCount);

    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = box.Vertices[i].Position;
        vertices[k].Normal = box.Vertices[i].Normal;
        vertices[k].Tex = box.Vertices[i].TexC;
        vertices[k].TangentU = XMFLOAT4(
            box.Vertices[i].TangentU.x,
            box.Vertices[i].TangentU.y,
            box.Vertices[i].TangentU.z,
            1.0f);
    }

    for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = grid.Vertices[i].Position;
        vertices[k].Normal = grid.Vertices[i].Normal;
        vertices[k].Tex = grid.Vertices[i].TexC;
        vertices[k].TangentU = XMFLOAT4(
            grid.Vertices[i].TangentU.x,
            grid.Vertices[i].TangentU.y,
            grid.Vertices[i].TangentU.z,
            1.0f);
    }

    for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = sphere.Vertices[i].Position;
        vertices[k].Normal = sphere.Vertices[i].Normal;
        vertices[k].Tex = sphere.Vertices[i].TexC;
        vertices[k].TangentU = XMFLOAT4(
            sphere.Vertices[i].TangentU.x,
            sphere.Vertices[i].TangentU.y,
            sphere.Vertices[i].TangentU.z,
            1.0f);
    }

    for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = cylinder.Vertices[i].Position;
        vertices[k].Normal = cylinder.Vertices[i].Normal;
        vertices[k].Tex = cylinder.Vertices[i].TexC;
        vertices[k].TangentU = XMFLOAT4(
            cylinder.Vertices[i].TangentU.x,
            cylinder.Vertices[i].TangentU.y,
            cylinder.Vertices[i].TangentU.z,
            1.0f);
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::PosNormalTexTan) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &mShapesVB));

    //
    // 인덱스도 정점과 같은 방식으로 몰아넣음
    //

    std::vector<UINT> indices;
    indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
    indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
    indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
    indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(device->CreateBuffer(&ibd, &iinitData, &mShapesIB));
}

void Game::BuildSkullGeometryBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    
    std::ifstream fin("Models/skull.txt");

    if (!fin)
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

    std::vector<Vertex::Basic32> vertices(vcount);
    for (UINT i = 0; i < vcount; ++i)
    {
        fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
        fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
    }

    fin >> ignore;
    fin >> ignore;
    fin >> ignore;

    mSkullIndexCount = 3 * tcount;
    std::vector<UINT> indices(mSkullIndexCount);
    for (UINT i = 0; i < tcount; ++i)
    {
        fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
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
    HR(device->CreateBuffer(&vbd, &vinitData, &mSkullVB));

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
    HR(device->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}

// 전체 화면 덮는 사각형 만들기
void Game::BuildScreenQuadGeometryBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    
    GeometryGenerator::MeshData quad;

    GeometryGenerator geoGen;
    geoGen.CreateFullscreenQuad(quad);

    std::vector<Vertex::Basic32> vertices(quad.Vertices.size());

    for (UINT i = 0; i < quad.Vertices.size(); ++i)
    {
        vertices[i].Pos = quad.Vertices[i].Position;
        vertices[i].Normal = quad.Vertices[i].Normal;
        vertices[i].Tex = quad.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * quad.Vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &quad.Indices[0];
    HR(device->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}

#pragma region Frame Update
// 기본적인 게임 루프 순회
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

// 게임 로직 내부 값 갱신
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: 여기에 게임 로직 추가.

    // 윈도우 창 프레임 표시
    std::wostringstream outs;
    outs.precision(6);
    outs << L"NC 인턴 지원자: 이경준" << L"    " // 게임 유저들에게는 초당 프레임수가 가시적이겠지만
        << L"FPS: " << timer.GetFramesPerSecond() << L"    " // 개발자에게는 프레임당 초가 도움이 더 된다.
        << L"Frame Time: " << 1000.f / timer.GetFramesPerSecond() << L" (ms)";
    SetWindowText(m_deviceResources->GetWindow(), outs.str().c_str());
    // 윈도우 창 프레임 표시

    if (GetAsyncKeyState('2') & 0x8000)
        mTestInst_1.mModel = mTest;
    if (GetAsyncKeyState('3') & 0x8000)
        mTestInst_1.mModel = mTestModels[0];
    if (GetAsyncKeyState('4') & 0x8000)
        mTestInst_1.mModel = mTestModels[1];
    if (GetAsyncKeyState('5') & 0x8000)
        mTestInst_1.mModel = mTestModels[2];
    if (GetAsyncKeyState('6') & 0x8000)
        mTestInst_1.mModel = mTestModels[3];
    
    bool check = false; 
    if (GetAsyncKeyState('S') & 0x8000)
    {
        mKeyInput[0] = 1;
    }
    if (GetAsyncKeyState('W') & 0x8000)
    {
        mKeyInput[1] = 1;
    }
    if (GetAsyncKeyState('D') & 0x8000)
    {
        mKeyInput[2] = 1;
    }
    if (GetAsyncKeyState('A') & 0x8000)
    {
        mKeyInput[3] = 1;
    }
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
    {
        mKeyInput[4] = 1;       
    }

    XMVECTOR zAxis = XMVectorSet(0, 0, 1, 0);
    XMVECTOR yAxis = XMVectorSet(0, 1, 0, 0);
    XMFLOAT3 cL = mCam.GetLook(); cL.y = 0;
    XMVECTOR cLook = XMLoadFloat3(&cL);
    XMVECTOR R = XMVector3AngleBetweenVectors(zAxis, cLook);
    XMVECTOR Cross = XMVector3Cross(zAxis, cLook);
    XMVECTOR Dot = XMVector3Dot(yAxis, Cross);
    float Rad = XMVectorGetX(R);
    if (XMVectorGetX(Dot) < 0)
        Rad *= -1;
    
    cLook = XMVector3Normalize(cLook);
    XMStoreFloat3(&cL, cLook);

    XMMATRIX a = XMMatrixAffineTransformation(XMVectorSet(1, 1, 1, 0), XMVectorSet(0, 0, 0, 1), XMVectorSet(0, 0, 0, 1), XMVectorSet(0, 0, 0, 1));

    XMFLOAT3 Right = mCam.GetRight();

    if (mKeyInput[0]) {
        mTestInst_1.mClipName = "walk";
        mTestInst_1.mSpeed = 7.f;
        mTestInst_1.mState = 1;
        mTestInst_1.mAngle = Rad;
        check = 1;
        if (mKeyInput[2]) {
            mTestInst_1.mAngle -= XM_PIDIV4;
            cLook = XMVector3Rotate(cLook, XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), -XM_PIDIV4));
            XMStoreFloat3(&cL, cLook);
        }
        else if (mKeyInput[3]) {
            mTestInst_1.mAngle += XM_PIDIV4;
            cLook = XMVector3Rotate(cLook, XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XM_PIDIV4));
            XMStoreFloat3(&cL, cLook);
        }
        if (mKeyInput[4]) {
            mTestInst_1.mClipName = "run";
            mTestInst_1.mSpeed = 14.f;
            mTestInst_1.mState = 2;
        }
        mTestInst_1.mZ -= elapsedTime * mTestInst_1.mSpeed * cL.z;
        mTestInst_1.mX -= elapsedTime * mTestInst_1.mSpeed * cL.x;
    }
    else if (mKeyInput[1]) {
        mTestInst_1.mClipName = "walk";
        mTestInst_1.mSpeed = 7.f;
        mTestInst_1.mState = 1;
        mTestInst_1.mAngle = Rad + XM_PI;
        check = 1;
        if (mKeyInput[2]) {
            mTestInst_1.mAngle += XM_PIDIV4;
            cLook = XMVector3Rotate(cLook, XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XM_PIDIV4));
            XMStoreFloat3(&cL, cLook);
        }
        else if (mKeyInput[3]) {
            mTestInst_1.mAngle -= XM_PIDIV4;
            cLook = XMVector3Rotate(cLook, XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), -XM_PIDIV4));
            XMStoreFloat3(&cL, cLook);
        }
        if (mKeyInput[4]) {
            mTestInst_1.mClipName = "run";
            mTestInst_1.mSpeed = 14.f;
            mTestInst_1.mState = 2;
        }
        mTestInst_1.mX += elapsedTime * mTestInst_1.mSpeed * cL.x;
        mTestInst_1.mZ += elapsedTime * mTestInst_1.mSpeed * cL.z;
    }
    else if (mKeyInput[2]) {
        mTestInst_1.mClipName = "walk";
        mTestInst_1.mSpeed = 7.f;
        mTestInst_1.mState = 1;
        mTestInst_1.mAngle = Rad + XM_PIDIV2 * 3;
        check = 1;
        if (mKeyInput[4]) {
            mTestInst_1.mClipName = "run";
            mTestInst_1.mSpeed = 14.f;
            mTestInst_1.mState = 2;
        }
        mTestInst_1.mX += elapsedTime * mTestInst_1.mSpeed * Right.x;
        mTestInst_1.mZ += elapsedTime * mTestInst_1.mSpeed * Right.z;
    }
    else if (mKeyInput[3]) {
        mTestInst_1.mClipName = "walk";
        mTestInst_1.mSpeed = 7.f;
        mTestInst_1.mState = 1;
        mTestInst_1.mAngle = Rad + XM_PIDIV2;
        check = 1;
        if (mKeyInput[4]) {
            mTestInst_1.mClipName = "run";
            mTestInst_1.mSpeed = 14.f;
            mTestInst_1.mState = 2;
        }
        mTestInst_1.mX -= elapsedTime * mTestInst_1.mSpeed * Right.x;
        mTestInst_1.mZ -= elapsedTime * mTestInst_1.mSpeed * Right.z;
    }

    if (!check) {
        mTestInst_1.mState = 5;
        mTestInst_1.mClipName = "idle";
    }
    
    XMFLOAT3 S = { mTestInst_1.mScale,mTestInst_1.mScale,mTestInst_1.mScale };
    XMFLOAT3 T = { mTestInst_1.mX, mTestInst_1.mY, mTestInst_1.mZ };
    XMMATRIX Result = XMMatrixAffineTransformation(XMLoadFloat3(&S), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
        XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), mTestInst_1.mAngle), XMLoadFloat3(&T));
    XMStoreFloat4x4(&mTestInst_1.mWorld, Result);

    mTestInst_1.Update(elapsedTime);

    BuildShadowTransform();

    XMFLOAT3 L = mCam.GetLook();
    mCamCenter = { mTestInst_1.mX, mTestInst_1.mY + 5, mTestInst_1.mZ };
    mCam.SetPosition(mCamCenter.x - L.x * mCamDist, mCamCenter.y - L.y * mCamDist, mCamCenter.z - L.z * mCamDist);

    mCam.UpdateViewMatrix();

    memset(mKeyInput, 0, sizeof(mKeyInput));

    elapsedTime;
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // 업데이트를 한번도 하지않았으면 렌더하지 말게 만듦
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto DSView = m_deviceResources->GetDepthStencilView();
    auto RTView = m_deviceResources->GetRenderTargetView();
    auto ScreenVP = m_deviceResources->GetScreenViewport();

    // TODO: 렌더링 코드를 여기에 작성.
    mSmap->BindDsvAndSetNullRenderTarget(context);

    DrawSceneToShadowMap();

    context->RSSetState(0);

    context->ClearDepthStencilView(DSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->RSSetViewports(1, &ScreenVP);
    mSsao->SetNormalDepthRenderTarget(DSView);

    DrawSceneToSsaoNormalDepthMap();

    mSsao->ComputeSsao(mCam);
    mSsao->BlurAmbientMap(2);

    //
    // Restore the back and depth buffer and viewport to the OM stage.
    //
    ID3D11RenderTargetView* renderTargets[1] = { RTView };
    context->OMSetRenderTargets(1, renderTargets, DSView);
    context->RSSetViewports(1, &ScreenVP);

    context->ClearRenderTargetView(RTView, reinterpret_cast<const float*>(&Colors::Silver));

    context->OMSetDepthStencilState(RenderStates::EqualsDSS, 0);

    XMMATRIX view = mCam.View();
    XMMATRIX proj = mCam.Proj();
    XMMATRIX viewProj = mCam.ViewProj();

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Set per frame constants.
    Effects::BasicFX->SetDirLights(mDirLights);
    Effects::BasicFX->SetEyePosW(mCam.GetPosition());
    Effects::BasicFX->SetCubeMap(mSky->CubeMapSRV());
    Effects::BasicFX->SetShadowMap(mSmap->DepthMapSRV());
    Effects::BasicFX->SetSsaoMap(mSsao->AmbientSRV());

    Effects::NormalMapFX->SetDirLights(mDirLights);
    Effects::NormalMapFX->SetEyePosW(mCam.GetPosition());
    Effects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());
    Effects::NormalMapFX->SetShadowMap(mSmap->DepthMapSRV());
    Effects::NormalMapFX->SetSsaoMap(mSsao->AmbientSRV());

    // Figure out which technique to use for different geometry.
    ID3DX11EffectTechnique* activeTech = Effects::NormalMapFX->Light3TexTech;
    ID3DX11EffectTechnique* activeSphereTech = Effects::BasicFX->Light3ReflectTech;
    ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light3ReflectTech;
    ID3DX11EffectTechnique* activeSkinnedTech = Effects::NormalMapFX->Light3TexSkinnedTech;

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    XMMATRIX world;
    XMMATRIX worldInvTranspose;
    XMMATRIX worldViewProj;

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    XMMATRIX toTexSpace(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

    //
    // Draw the grid, cylinders, and box without any cubemap reflection.
    // 

    UINT stride = sizeof(Vertex::PosNormalTexTan);
    UINT offset = 0;

    context->IASetInputLayout(InputLayouts::PosNormalTexTan);
    context->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
    context->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

    if (GetAsyncKeyState('1') & 0x8000)
        context->RSSetState(RenderStates::WireframeRS);

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // Draw the grid.
        world = XMLoadFloat4x4(&mGridWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::NormalMapFX->SetWorld(world);
        Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
        Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj * toTexSpace);
        Effects::NormalMapFX->SetShadowTransform(world * shadowTransform);
        Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
        Effects::NormalMapFX->SetMaterial(mGridMat);
        Effects::NormalMapFX->SetDiffuseMap(mStoneTexSRV);
        Effects::NormalMapFX->SetNormalMap(mStoneNormalTexSRV);
        Effects::NormalMapFX->SetSpecularMap(nullptr);

        activeTech->GetPassByIndex(p)->Apply(0, context);
        context->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);      
    }

    //
    // 애니메이션 되는 캐릭터 그리기
    //

    context->RSSetState(RenderStates::NoCullRS);

    if (GetAsyncKeyState('1') & 0x8000)
        context->RSSetState(RenderStates::WireframeRS);

    context->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

    activeSkinnedTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        world = XMLoadFloat4x4(&mTestInst_1.mWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::NormalMapFX->SetWorld(world);
        Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
        Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj * toTexSpace);
        Effects::NormalMapFX->SetShadowTransform(world * shadowTransform);
        Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
        Effects::NormalMapFX->SetBoneTransforms(
            &mTestInst_1.mFinalTransforms[0],
            mTestInst_1.mFinalTransforms.size());

        for (UINT subset = 0; subset < mTestInst_1.mModel->mSubsetCount; ++subset)
        {
            Effects::NormalMapFX->SetMaterial(mTestInst_1.mModel->mMat[subset]);
            Effects::NormalMapFX->SetDiffuseMap(mTestInst_1.mModel->mDiffuseMapSRV[subset]);
            Effects::NormalMapFX->SetNormalMap(mTestInst_1.mModel->mNormalMapSRV[subset]);
            Effects::NormalMapFX->SetSpecularMap(mTestInst_1.mModel->mSpecularMapSRV[subset]);

            activeSkinnedTech->GetPassByIndex(p)->Apply(0, context);
            mTestInst_1.mModel->mMeshes.Draw(context, subset);
        }      
    }

    // Turn off wireframe.
    context->RSSetState(0);

    // Restore from RenderStates::EqualsDSS
    context->OMSetDepthStencilState(0, 0);

    // Debug view SSAO map.
    DrawScreenQuad(mSsao->AmbientSRV());

    mSky->Draw(context, mCam);

    // restore default states, as the SkyFX changes them in the effect file.
    context->RSSetState(0);
    context->OMSetDepthStencilState(0, 0);

    // Unbind shadow map and AmbientMap as a shader input because we are going to render
    // to it next frame.  These textures can be at any slot, so clear all slots.
    ID3D11ShaderResourceView* nullSRV[16] = { 0 };
    context->PSSetShaderResources(0, 16, nullSRV);

    context;

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CadetBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

void Game::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(m_deviceResources->GetWindow());
}

void Game::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Game::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        mCam.SetPosition(mCamCenter);

        mCam.Pitch(dy);
        XMVECTOR zAxis = XMVectorSet(0, 0, 1, 0);
        XMVECTOR yAxis = XMVectorSet(0, 1, 0, 0);
        XMVECTOR cLook = mCam.GetLookXM();
        XMVECTOR cRight = mCam.GetRightXM();
        XMVECTOR Cross = XMVector3Cross(yAxis, cLook);
        XMVECTOR Dot = XMVector3Dot(cRight, Cross);
        if (XMVectorGetX(Dot) < 0)
            mCam.Pitch(-dy);

        mCam.RotateY(dx);
        XMFLOAT3 L = mCam.GetLook();
        mCam.SetPosition(mCamCenter.x - L.x * mCamDist, mCamCenter.y - L.y * mCamDist, mCamCenter.z - L.z * mCamDist);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    mCam.SetLens(0.25f * MathHelper::Pi, m_deviceResources->GetAspectRatio(), 1.0f, 1000.0f);
    
    if (mSsao)
    {
        mSsao->OnSize(m_deviceResources->GetClientWidth(), m_deviceResources->GetClientHeight(), mCam.GetFovY(), mCam.GetFarZ());
    }
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
