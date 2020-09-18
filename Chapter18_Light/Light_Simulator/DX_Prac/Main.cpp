#include <windows.h>
#include <d3d9.h>
#include <d3dx9math.h>
#include "CGameEdu01.h"

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"DxErr.lib")
#pragma comment (lib,"d3dxof.lib")
#pragma comment (lib,"winmm.lib")

#define MAX_LOADSTRING 100

CGameEdu01 Mechanism;

// 전역 변수:
HINSTANCE hInst;								// 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];					// 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];			// 기본 창 클래스 이름입니다.
HWND g_hWnd, g_hMenuWnd; // 메인 윈도우, 다이얼로그 윈도우 핸들
int g_nDlgWidth, g_nDlgHeight; // 다이얼로그 가로, 세로 크기

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg,
	WPARAM wParam, LPARAM lParam);

LPCTSTR lpszClass = TEXT("DX Sample"); // 타이틀 이름

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

	hInst = hInstance;

	g_hWnd = hwnd = CreateWindow(lpszClass, //윈도우가 생성되면 핸들(hwnd)이 반환
		lpszClass, //윈도우 클래스, 타이틀 이름
		WS_OVERLAPPEDWINDOW, //윈도우 스타일
		CW_USEDEFAULT, //윈도우 위치, x좌표
		CW_USEDEFAULT, //윈도우 위치, y좌표
		900, //윈도우 폭   
		900, //윈도우 높이   
		NULL, //부모 윈도우 핸들 
		NULL, //메뉴 핸들
		hInstance,     //응용 프로그램 ID
		NULL      //생성된 윈도우 정보
	);
	ShowWindow(hwnd, nCmdShow); //윈도우의 화면 출력
	UpdateWindow(hwnd); //O/S 에 WM_PAINT 메시지 전송

	Mechanism.D3DInit(g_hWnd);

	// 컨트롤 값 초기화 
	/*
	char string[10];
	wsprintf(string, "%.1f", g_GameEdu01.m_Material.Diffuse.r);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT1, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Material.Diffuse.g);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT2, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Material.Diffuse.b);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT3, string);

	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Diffuse.r);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT4, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Diffuse.g);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT5, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Diffuse.b);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT6, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Specular.r);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT7, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Specular.g);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT8, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Specular.b);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT9, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Ambient.r);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT10, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Ambient.g);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT11, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Ambient.b);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT12, string);

	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Direction.x);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT13, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Direction.y);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT14, string);
	wsprintf(string, "%.1f", g_GameEdu01.m_Light.Direction.z);
	SetDlgItemText(g_hMenuWnd, IDC_EDIT15, string);
	*/

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