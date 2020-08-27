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

void PrintMat(D3DXMATRIX *buffer)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			printf("%.2f	", buffer->m[i][j]);
		}
		cout << endl;
	}
}

int main()
{
	D3DXMATRIX matIdentity, matMatrix, matResult;
	D3DXMatrixIdentity(&matIdentity); // matIdentity를 단위 행렬로 만든다.
	for (int i = 0;i < 4;i++)
	{
		for (int j = 0; j < 4; j++)
		{
			matMatrix(i, j) = float(i * 4 + j + 1);
		}
	}
	cout << "Mat_1" << endl;
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "Mat_2" << endl;
	PrintMat(&matIdentity); cout << "\n\n";

	cout << "Mat_1 * Mat_2" << endl;
	matResult = matIdentity * matMatrix;
	PrintMat(&matResult); cout << "\n\n";

	cout << "Mat_1 + Mat_2" << endl;
	matResult = matIdentity + matMatrix;
	PrintMat(&matResult); cout << "\n\n";

	cout << "Mat_1의 전치행렬" << endl;
	D3DXMatrixTranspose(&matResult, &matMatrix);
	PrintMat(&matResult); cout << "\n\n";

	cout << "Mat_1의 회전행렬" << endl;
	// X축을 기준으로 회전을 한 행렬을 구하는 함수
	D3DXMatrixRotationX(&matMatrix, 0.3f); // 회전 행렬에 대해서는 무조건 역행렬이 존재 하기에 사용
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "위 회전행렬의 역행렬" << endl;
	D3DXMatrixInverse(&matResult, NULL, &matMatrix);
	PrintMat(&matResult); cout << "\n\n";
}