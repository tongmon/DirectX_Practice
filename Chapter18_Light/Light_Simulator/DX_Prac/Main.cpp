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

// ���� ����:
HINSTANCE hInst;								// ���� �ν��Ͻ��Դϴ�.
TCHAR szTitle[MAX_LOADSTRING];					// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR szWindowClass[MAX_LOADSTRING];			// �⺻ â Ŭ���� �̸��Դϴ�.
HWND g_hWnd, g_hMenuWnd; // ���� ������, ���̾�α� ������ �ڵ�
int g_nDlgWidth, g_nDlgHeight; // ���̾�α� ����, ���� ũ��

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg,
	WPARAM wParam, LPARAM lParam);

LPCTSTR lpszClass = TEXT("DX Sample"); // Ÿ��Ʋ �̸�

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{ //szCmdLine : Ŀ��Ʈ���� �󿡼� ���α׷� ���� �� ���޵� ���ڿ�
	HWND hwnd; //iCmdShow : �����찡 ȭ�鿡 ��µ� ����
	MSG msg;
	WNDCLASS WndClass; //WndClass ��� ����ü ���� 
	WndClass.style = CS_HREDRAW | CS_VREDRAW; //��½�Ÿ�� : ����/������ ��ȭ�� �ٽ� �׸�
	WndClass.lpfnWndProc = WndProc; //���ν��� �Լ���
	WndClass.cbClsExtra = 0; //O/S ��� ���� �޸� (Class)
	WndClass.cbWndExtra = 0; //O/s ��� ���� �޸� (Window)
	WndClass.hInstance = hInstance; //���� ���α׷� ID
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); //������ ����
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW); //Ŀ�� ����
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//����   
	WndClass.lpszMenuName = NULL; //�޴� �̸�
	WndClass.lpszClassName = lpszClass; //Ŭ���� �̸�
	RegisterClass(&WndClass); //�ռ� ������ ������ Ŭ������ �ּ�

	hInst = hInstance;

	g_hWnd = hwnd = CreateWindow(lpszClass, //�����찡 �����Ǹ� �ڵ�(hwnd)�� ��ȯ
		lpszClass, //������ Ŭ����, Ÿ��Ʋ �̸�
		WS_OVERLAPPEDWINDOW, //������ ��Ÿ��
		CW_USEDEFAULT, //������ ��ġ, x��ǥ
		CW_USEDEFAULT, //������ ��ġ, y��ǥ
		900, //������ ��   
		900, //������ ����   
		NULL, //�θ� ������ �ڵ� 
		NULL, //�޴� �ڵ�
		hInstance,     //���� ���α׷� ID
		NULL      //������ ������ ����
	);
	ShowWindow(hwnd, nCmdShow); //�������� ȭ�� ���
	UpdateWindow(hwnd); //O/S �� WM_PAINT �޽��� ����

	Mechanism.D3DInit(g_hWnd);

	// ��Ʈ�� �� �ʱ�ȭ 
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

	// ���� �޽��� �����Դϴ�:
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
	return DefWindowProc(hwnd, iMsg, wParam, lParam); //CASE���� ���ǵ��� ���� �޽����� Ŀ���� ó���ϵ��� �޽��� ����
}