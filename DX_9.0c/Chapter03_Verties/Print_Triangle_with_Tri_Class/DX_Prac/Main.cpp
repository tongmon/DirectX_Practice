#include <windows.h>
#include <d3d9.h>
#include "CGameEdu01.h"

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"DxErr.lib")
#pragma comment (lib,"d3dxof.lib")
#pragma comment (lib,"winmm.lib")

// 삼각형을 그릴 경우 따로 삼각형 클래스를 만들어 CGameEdu01 클래스 내의 멤버변수로 활용하는 방식
// 실제 이 방식이 따로 클래스 만들지 않고 때려박는 것보다 현업에서 많이 쓰이고 효율적이다.
// 이 코드를 보면서 실제 프레임워크가 어떻게 효율적으로 구현되는지 알아낼 수 있다.
// 실 구현자는 삼각형을 그릴 때 그 그리는 것에만 집중하면 된다. 
// 디바이스를 어떻게 할당, 해제 뒤지고 볶는 것은 프레임 워크가 해준다.
CGameEdu01 Mechanism;

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg,
	WPARAM wParam, LPARAM lParam);

LPCTSTR lpszClass = TEXT("Api Sample"); // 타이틀 이름
HWND g_hWnd;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{ //szCmdLine : 커멘트라인 상에서 프로그램 구동 시 전달된 문자열
	HWND hwnd; //iCmdShow : 윈도우가 화면에 출력될 형태
	MSG msg;
	WNDCLASS WndClass; //WndClass 라는 구조체 정의 
	WndClass.style = CS_HREDRAW | CS_VREDRAW; //출력스타일 : 수직/수평의 변화시 다시 그림
	WndClass.lpfnWndProc = WndProc; //프로시저 함수명
	WndClass.cbClsExtra = 0; //O/S 사용 여분 메모리 (Class)
	WndClass.cbWndExtra = 0; //O/s 사용 여분 메모리 (Window)
	WndClass.hInstance = hInstance; //응용 프로그램 ID
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); //아이콘 유형
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW); //커서 유형
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//배경색   
	WndClass.lpszMenuName = NULL; //메뉴 이름
	WndClass.lpszClassName = lpszClass; //클래스 이름
	RegisterClass(&WndClass); //앞서 정의한 윈도우 클래스의 주소

	g_hWnd = hwnd = CreateWindow(lpszClass, //윈도우가 생성되면 핸들(hwnd)이 반환
		lpszClass, //윈도우 클래스, 타이틀 이름
		WS_OVERLAPPEDWINDOW, //윈도우 스타일
		CW_USEDEFAULT, //윈도우 위치, x좌표
		CW_USEDEFAULT, //윈도우 위치, y좌표
		CW_USEDEFAULT, //윈도우 폭   
		CW_USEDEFAULT, //윈도우 높이   
		NULL, //부모 윈도우 핸들 
		NULL, //메뉴 핸들
		hInstance,     //응용 프로그램 ID
		NULL      //생성된 윈도우 정보
	);
	ShowWindow(hwnd, nCmdShow); //윈도우의 화면 출력
	UpdateWindow(hwnd); //O/S 에 WM_PAINT 메시지 전송

	Mechanism.D3DInit(g_hWnd);

	// 게임 메시지 루프입니다:
	while (true)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Update and Render
			Mechanism.Update();
			Mechanism.Render();
		}
	}
	return (int)msg.wParam;
}

HDC hdc;


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

	//PAINTSTRUCT ps;

	switch (iMsg)
	{
	case WM_CREATE:
		break;

	case WM_PAINT:
		Mechanism.Render();
		break;

	case WM_DESTROY:
		Mechanism.Cleanup();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam); //CASE에서 정의되지 않은 메시지는 커널이 처리하도록 메시지 전달
}