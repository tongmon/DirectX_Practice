// Directx11 템플릿이다.
// VS2019 x86, x64에서 모두 오류없이 작동한다.
// 유니코드에서 작동하며 사전에 준비해야 할 것은 Effects11d.lib, Effects11.lib
// 이 두개의 라이브러리 파일 뿐이다.
// 부족한 기능들은 이 뼈대에 붙여나가면 되며, DXTK에 강력한 기능들이 들어있으니
// 학습하여 사용하면 된다. (GLUT과 같은 기능을 지원하는 DXUT는 잘 작동 안해서 뺌)

//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "d3dx11effect.h"
#include "d3dxGlobal.h"
#include "dxerr.h"

// 밑의 헤더들은 헤더 pch에 넣었다.
// pch를 미리 컴파일된 헤더로 설정하면 c1010 에러가 발생하기에
// 미리 컴파일된 헤더 기능을 꺼놓았다.
//#include <d3dcompiler.h>
//#include <windowsx.h>
//#include <sstream>

// Effects11d.lib, Effects11.lib 이 라이브러리는 사전에 Windows Kit에 들어가 있어야 한다.
// 사전에 라이브러리 준비하는 방법은 텍스쳐 파일 설명 참조
#if defined( DEBUG ) || defined( _DEBUG )
#pragma comment (lib,"Effects11d.lib")
#else
#pragma comment (lib,"Effects11.lib")
#endif

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

public:

    void BuildGeometryBuffers();
    void BuildFX();
    void BuildVertexLayout();

private:

    Microsoft::WRL::ComPtr<ID3D11Buffer> mBoxVB; // 정점 버퍼
    Microsoft::WRL::ComPtr<ID3D11Buffer> mBoxIB; // 인덱스 버퍼

    // 얘네들도 ComPtr 사용하면 편하지만 일단 사용안하고 써봤다.
    // Microsoft::WRL::ComPtr 스마트 포인터를 사용하지 않는 Com 객체이기에 소멸할 때에 맞춰서
    // Release 해주어야 메모리 누수가 없다.
    ID3DX11Effect* mFX;
    ID3DX11EffectTechnique* mTech;
    ID3DX11EffectMatrixVariable* mfxWorldViewProj;

    Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;

    // XM이 붙은 DirectXmath 라이브러리를 사용하는 형태를 사용하려면
    // 헤더 파일에서는 using namespace DirectX 를 쓰지 않는 이상
    // DirectX:: 를 사용해서 명시를 해주어야 한다.
    // 안그러면 Windows kit에 있는 다른 헤더와 충돌하여 오류난다.
    DirectX::XMFLOAT4X4 mWorld; // 월드 행렬, SRT
    DirectX::XMFLOAT4X4 mView; // 카메라 위치
    DirectX::XMFLOAT4X4 mProj; // 투영 행렬

    float mTheta;
    float mPhi;
    float mRadius;

    POINT mLastMousePos;
};
