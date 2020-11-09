#pragma once

#include "d3dUtil.h"
#include "GameTimer.h"
#include <string>

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE AppInst()const; // 응용 프로그램 인스턴스 핸들의 복사본을 돌려주는 함수
	HWND      MainWnd()const; // 메인 윈도우 핸들의 복사본을 돌려주는 함수
	float     AspectRatio()const; // 높이에 대한 너비의 비율을 도출해주는 함수(종횡비)

	int Run(); // 윈도우 루프, 실제 게임 구동 루프라고 보면 된다.

	// 프레임 워크 메서드들로 파생 클라이언트 클래스는 이 메서드들을 입맛대로 재정의하여 쓰면된다.

	// 말 그대로 초기화 함수, 윈도우 창 초기화, DX 초기화 이외에도 자식 클래스에서 재정의해서 초기화 변수들을 집어 넣으면 된다.
	virtual bool Init();
	// 창 크기 재조정시에 렌더 버퍼, 깊이_스탠실 버퍼의 크기 또한 재정의 해주어야 하는데 이를 담당
	virtual void OnResize();
	virtual void UpdateScene(float dt) = 0; // 말 그대로 실제 값의 변화가 이루어지는 업데이트 함수
	virtual void DrawScene() = 0; // 실제 보이는 것이 조정되는 그리기 함수
	// 특정 메시지를 윈도우에게 맡기지 않고 직접 응용프로그램에서 따로 처리하고 싶을 경우 호출 하는 함수
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// 마우스 입력 처리를 위한 편의용 가상 메서드들
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

protected:
	bool InitMainWindow(); // WIN32 메인 윈도우 초기화
	bool InitDirect3D(); // Direct3D 11을 초기화 하는 함수

	void CalculateFrameStats(); // 초당 프레임, 평균 프레임당 밀리초를 계산하는 함수

protected:

	HINSTANCE mhAppInst; // 응용 프로그램 인스턴스 핸들
	HWND      mhMainWnd; // 메인 윈도우 핸들
	bool      mAppPaused; // 앱이 멈추었는지 판단
	bool      mMinimized; // 앱이 최소화 상태인지 판단
	bool      mMaximized; // 앱이 최대화 생태인지 판단
	bool      mResizing; // 사용자가 크기 조정용 테두리를 사용하는 상태인지 판단
	UINT      m4xMsaaQuality; // 4X MSAA 에일리어싱 품질 수준

	GameTimer mTimer; // 게임 시간 측정 클래스 선언

	// D3D11 장치, 버퍼 전환 교환 사슬
	// 깊이_스텐실 버퍼를 위한 2차원 텍스쳐, 렌더 대상용 2차원 텍스쳐
	// 깊이_스텐실 뷰, 뷰포트... 등에 대한 선언
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	// 예제에선 GPU에서 연산 작동하는 옵션인 D3D_DRIVER_TYPE_HARDWARE 로 타입을 지정한다.
	std::wstring mMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;
	int mClientWidth;
	int mClientHeight;
	bool mEnable4xMsaa;
};