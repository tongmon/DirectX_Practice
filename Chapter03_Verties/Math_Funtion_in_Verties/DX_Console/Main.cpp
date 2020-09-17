#include <iostream>
#include <d3d9.h>
#include <d3dx9math.h>
using namespace std;

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"DxErr.lib")
#pragma comment (lib,"d3dxof.lib")
#pragma comment (lib,"winmm.lib")

struct CUSTOMVERTEX // 정점 구조체 방법 1
{
	float x, y, z, rhw; // rhw는 2d 즉, 모니터 상의 좌표계를 통해서 정점을 나태 낼 때에 쓰인다.
	DWORD color;
};
#define D3DFVX_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE) // 정점 출력 모드 묶기

struct CUSTOMVERTEX_2 // 정점 구조체 방법 2
{
	D3DXVECTOR3 positon;
	D3DXVECTOR3 normal;
	DWORD diffuse; // 색상
	float tu, tv; // 정점에 텍스쳐를 입힐 경우 사용
};
#define D3DFVX_CUSTOMVERTEX_2 (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1) // 정점 출력 모드 묶기

int main()
{
	D3DXVECTOR3 v1(3.0f, 0.0f, 0.0f);
	D3DXVECTOR3 v2(0.0f, 3.0f, 0.0f);
	D3DXVECTOR3 v3;
	D3DXVECTOR3 v4(3.0f, 3.0f, 1.0f);

	v3 = v1 + v2;
	cout << "벡터의 덧셈\n" << v3.x << " " << v3.y << " " << v3.z << endl;

	D3DXVec3Add(&v3, &v1, &v2);
	cout << v3.x << " " << v3.y << " " << v3.z << "\n\n";

	v3 = v1 - v2;
	cout << "벡터의 뺄셈\n" << v3.x << " " << v3.y << " " << v3.z << endl;

	D3DXVec3Subtract(&v3, &v1, &v2);
	cout << v3.x << " " << v3.y << " " << v3.z << "\n\n";

	float Length = D3DXVec3Length(&v4);
	cout << "벡터(v4)의 크기: " << Length << "\n\n";

	D3DXVECTOR3 v5(1.0f, 2.0f, 0.0f); 
	cout << "벡터와 스칼라의 곱\n" << v5.x << " " << v5.y << " " << v5.z << endl;
	float Scale = 2.0f;
	D3DXVec3Scale(&v5, &v5, Scale);
	cout << v5.x << " " << v5.y << " " << v5.z << "\n\n";

	D3DXVECTOR3 v6(3.0f, 1.0f, 4.0f);
	D3DXVECTOR3 vResult;
	D3DXVec3Normalize(&vResult, &v6);
	float NormLength = D3DXVec3Length(&vResult);
	cout << "v6의 단위 벡터와 그 크기\n" << vResult.x << " " << vResult.y << " " << vResult.z << endl;
	cout << "v6 단위 벡터의 크기: " << NormLength << "\n\n";

	// 내적은 두 벡터 사이의 사잇각 즉, 코사인 값을 알아낼 때 쓰인다.
	D3DXVECTOR3 v7(3.0f, 1.0f, 4.0f);
	D3DXVECTOR3 v8(2.0f, 7.0f, 1.0f);
	float Dot = D3DXVec3Dot(&v7, &v8); // 내적
	float v7_len = D3DXVec3Length(&v7);
	float v8_len = D3DXVec3Length(&v8);
	cout << "두 벡터 v7과 v8의 내적: " << Dot << endl;
	cout << "두 벡터 v7과 v8 사이의 사잇각(Cosin) 라디안: " << Dot / (v7_len * v8_len) << "\n\n";

	// 외적은 두 벡터가 이루어 만들어 지는 평면에 수직을 이루는 법선 벡터를 도출할 때 쓰인다.
	D3DXVec3Cross(&vResult, &v1, &v2); // v1과 v2 순서에 유의하라. 순서가 바뀐다면 반대 방향 법선 벡터가 도출된다.
	D3DXVec3Normalize(&vResult, &vResult);
	cout << "v1과 v2의 법선 벡터\n" << vResult.x << " " << vResult.y << " " << vResult.z << "\n";

	// 정점 삽입시에 왠만하면 시계방향 순으로 삽입할 것!
	// 이유는 각종 함수에 삽입시에 순서에 따라 도출되는 벡터의 방향이 바뀔 수가 있다.
	// 시계(CW), 반시계(CCW) 방향을 잘 구분하자!
	CUSTOMVERTEX vertices[] =
	{
		{150.0f, 50.0f, 0.5f, 1.0f, 0xffff0000},
		{250.0f, 250.0f, 0.5f, 1.0f, 0xff00ff00},
		{50.0f, 250.0f, 0.5f, 1.0f, 0xff00ffff}
	};
}