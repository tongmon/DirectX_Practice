//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

struct Vertex // 정점 정의
{
    XMFLOAT3 Pos; // 위치
    XMFLOAT4 Color; // 색상
};

Game::Game() noexcept(false) : mBoxVB(0), mBoxIB(0), mFX(0), mTech(0), mfxWorldViewProj(0), mInputLayout(0)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mTheta = 1.5f * XM_PI;
    mPhi = 0.25f * XM_PI;
    mRadius = 5.0f;

    // XMStoreFloat4x4 ---> XMMATRIX 형을 XMFloat4x4 형에 저장
    XMMATRIX I = XMMatrixIdentity(); // 단위 행렬로 초기화
    XMStoreFloat4x4(&mWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);
}

Game::~Game()
{
    // Com 해체
    mFX->Release();
    mTech->Release();
    mfxWorldViewProj->Release();
}

// 시작시에 Direct3D 자원 초기화
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    BuildGeometryBuffers(); // 정점 정의
    BuildFX(); // FX 파일 읽어서 쉐이딩
    BuildVertexLayout(); // 입력 배치 설정 함수

    // TODO: 타이머 설정을 원하면 바꿀 수 있다.
    // 밑의 예시는 60 프레임 고정으로 설정하는 방법이다.
    /*
    m_timer.SetFixedTimeStep(true); // 고정 프레임 사용
    m_timer.SetTargetElapsedSeconds(1.0 / 60); // 60 프레임 고정
    */
}

void Game::BuildGeometryBuffers() // 정점 정의 함수
{
    auto device = m_deviceResources->GetD3DDevice();

    XMFLOAT4 Col[8];
    XMStoreFloat4(&Col[0], Colors::White); XMStoreFloat4(&Col[1], Colors::Black);
    XMStoreFloat4(&Col[2], Colors::Red); XMStoreFloat4(&Col[3], Colors::Green);
    XMStoreFloat4(&Col[4], Colors::Blue); XMStoreFloat4(&Col[5], Colors::Yellow);
    XMStoreFloat4(&Col[6], Colors::Cyan); XMStoreFloat4(&Col[7], Colors::Magenta);

    // 정점들 정의
    Vertex vertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), Col[0] },
        { XMFLOAT3(-1.0f, +1.0f, -1.0f), Col[1] },
        { XMFLOAT3(+1.0f, +1.0f, -1.0f), Col[2] },
        { XMFLOAT3(+1.0f, -1.0f, -1.0f), Col[3] },
        { XMFLOAT3(-1.0f, -1.0f, +1.0f), Col[4] },
        { XMFLOAT3(-1.0f, +1.0f, +1.0f), Col[5] },
        { XMFLOAT3(+1.0f, +1.0f, +1.0f), Col[6] },
        { XMFLOAT3(+1.0f, -1.0f, +1.0f), Col[7] }
    };

    D3D11_BUFFER_DESC vbd;
    // 보통은 D3D11_USAGE_DEFAULT를 해서 GPU가 자원을 읽고 쓰게 하지만 
    // 여기선 최적화를 위해 버퍼 생성 후에 전혀 변경이 없는 D3D11_USAGE_IMMUTABLE를 사용
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * 8; // 생성할 정점 버퍼의 크기
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 정점 버퍼는 무조건 이거 쓴다.
    vbd.CPUAccessFlags = 0; // CPU가 정점 버퍼 계산에 관여하지 않으므로 0
    vbd.MiscFlags = 0; // 정점 버퍼에서는 이건 무조건 0
    vbd.StructureByteStride = 0; // 구조적 버퍼 사용 안하면 0
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices;
    HR(device->CreateBuffer(&vbd, &vinitData, mBoxVB.GetAddressOf())); // vbd(정점 버퍼 속성), vinitData(정점 초기화), mBoxVB(생성된 정점이 담김)

    // 인덱스 버퍼 생성, 시계방향이 DX에서 앞면
    UINT indices[] = {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };

    // 인덱스 버퍼 특성 지정, 방식은 정점 버퍼와 비슷
    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * 36; // 36개의 인덱스
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; // 인덱스 버퍼는 이렇게 지정
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA iinitData; // 색인 버퍼 초기화할 자료를 지정
    iinitData.pSysMem = indices;
    HR(device->CreateBuffer(&ibd, &iinitData, mBoxIB.GetAddressOf())); // 색인 버퍼 생성, HR은 디버그 용도
}

void Game::BuildFX()
{
    auto device = m_deviceResources->GetD3DDevice();
    
    DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
    shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

    ID3D10Blob* compiledShader = 0;
    ID3D10Blob* compilationMsgs = 0;
    HRESULT hr = D3DCompileFromFile(L"FX/color.fx", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "none", "fx_5_0", shaderFlags,
        0, &compiledShader, &compilationMsgs);

    if (FAILED(hr))
    {
        if (compilationMsgs)
        {
            OutputDebugStringA((char*)compilationMsgs->GetBufferPointer());
            compilationMsgs->Release();
        }

        if (compiledShader)
            compiledShader->Release();

        return;
    }

    HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
        0, device, &mFX)); // 이펙트를 mFX에 저장

    // 컴파일된 셰이더를 다 사용해서 해제함
    compiledShader->Release();

    mTech = mFX->GetTechniqueByName("ColorTech"); // fx 파일을 보면 ColorTech라는 이름의 테크님 구조체가 있다. 이를 가져오는 것
    mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix(); // fx 파일에서 gWorldViewProj 변수를 행렬 형태로 획득함
}

void Game::BuildVertexLayout()
{
    auto device = m_deviceResources->GetD3DDevice();

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
    D3DX11_PASS_DESC passDesc; // 패스를 서술하는 구조체, 입력 서명
    // Effect 객체마다 패스가 있고 해당 패스마다 연관된 정점 셰이더가 존재
    // 효과 객체로부터 패스를 서술하는 구조체 획득 가능
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(device->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
        passDesc.IAInputSignatureSize, &mInputLayout)); // 생성된 입력 배치는 mInputLayout 여기에 담기게 된다.
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.

    // 윈도우 창 프레임 표시
    std::wostringstream outs;
    outs.precision(6);
    outs << L"DX_Sample" << L"    " // 게임 유저들에게는 초당 프레임수가 가시적이겠지만
        << L"FPS: " << timer.GetFramesPerSecond() << L"    " // 개발자에게는 프레임당 초가 도움이 더 된다.
        << L"Frame Time: " << 1000.f / timer.GetFramesPerSecond() << L" (ms)";
    SetWindowText(m_deviceResources->GetWindow(), outs.str().c_str());
    // 윈도우 창 프레임 표시

    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    // 시야 행렬 구축
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f); // 카메라 위치, 좌표니까 1.0f
    XMVECTOR target = XMVectorZero(); // 원점을 바라봄
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 보통 카메라 기울기 설정(거의 고정)

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up); // 시야행렬 설정
    XMStoreFloat4x4(&mView, V); // mView = 시야 행렬

    elapsedTime;
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.

    context->IASetInputLayout(mInputLayout.Get()); // 입력 배치를 사용하고자 하는 장치에 묶기
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 삼각 리스트 형태의 방식으로

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, mBoxVB.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(mBoxIB.Get(), DXGI_FORMAT_R32_UINT, 0); // 인덱스 버퍼들을 입력 조립기 단계에 묶기

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
        mTech->GetPassByIndex(p)->Apply(0, context);

        // 36개의 박스 인덱스 버퍼들
        context->DrawIndexed(36, 0, 0); // 정점 인덱스 개수, 첫 정점 위치, 정점에 더해질 값을 지정해 실제로 그리기
    }

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
    if ((btnState & MK_LBUTTON) != 0) // 좌측 클릭으로 카메라 위치 이동
    {
        // 1픽셀이 4분의 1도 (Degree 단위) 가 되게 한다.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        // 마우스 입력에 기초한 각도로 상자 주변의 궤도 카메라 갱신
        mTheta += dx;
        mPhi += dy;

        // mPhi(사람 시점으로는 상하 고개 끄덕이는 거)의 각도 범위를 0 ~ 180으로 제한하는 것.
        mPhi = mPhi < 0.1f ? 0.1f : (mPhi > XM_PI - 0.1f ? XM_PI - 0.1f : mPhi);
    }
    else if ((btnState & MK_RBUTTON) != 0) // 우측 클릭으로 카메라 확대 축소
    {
        // 1픽셀이 장면의 0.005 단위가 되게 한다.
        float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

        // 마우스 입력에 기초하여 궤도 카메라의 반지름을 조정
        mRadius += dx - dy;

        // 반지름의 범위도 3 ~ 15로 제한을 둔다.
        mRadius = mRadius < 3.0f ? 3.0f : (mRadius > 15.0f ? 15.0f : mRadius);
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
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI, m_deviceResources->GetAspectRatio(), 1.0f, 1000.0f); // 투영행렬 설정
    XMStoreFloat4x4(&mProj, P); // 투영 행렬 갱신
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
