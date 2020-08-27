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
	D3DXMATRIX matMatrix;

	cout << "이동 행렬\n";
	// 모든 축으로 부터 0.5씩 이동되는 행렬 제작
	D3DXMatrixTranslation(&matMatrix, 0.5f, 0.5f, 0.5f); // 이제 이 행렬을 좌표와 곱해주면 좌표가 +0.5 이동하게 되는 것이다.
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "스케일링 행렬\n";
	// 벡터 좌표의 크기가 곱연산으로 변화되는 행렬 제작
	D3DXMatrixScaling(&matMatrix, 0.5f, 0.5f, 0.5f); // 이제 이 행렬을 좌표와 곱해주면 좌표 값이 절반으로 줄어든다.
	PrintMat(&matMatrix); cout << "\n\n";

	// 밑의 두 함수는 매우 많이 쓰는 함수이다.
	float Radian = 0.785398, Degree = 45;
	cout << "0.785398라디안을 호도법으로 변환: " << D3DXToDegree(Radian) << endl;
	cout << "45도를 라디안으로 변환: " << D3DXToRadian(Degree) << "\n\n";

	cout << "X축 회전 행렬\n";
	// 왼손 좌표계의 X축을 기준으로 회전 시켜주는 행렬 제작, 왼손 좌표계이기에 시계방향(CW)으로 회전 한다.
	// 값은 라디안으로 받는데 위의 호도법->라디안 변환 함수를 사용해서 전달해주었다.
	D3DXMatrixRotationX(&matMatrix, D3DXToRadian(45)); // 이제 이 행렬을 좌표와 곱해주면 좌표의 위치가 X축 기준 45도 회전된다.
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "Y축 회전 행렬\n";
	D3DXMatrixRotationY(&matMatrix, D3DXToRadian(45)); // 이제 이 행렬을 좌표와 곱해주면 좌표의 위치가 Y축 기준 45도 회전된다.
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "Z축 회전 행렬\n";
	D3DXMatrixRotationY(&matMatrix, D3DXToRadian(45)); // 이제 이 행렬을 좌표와 곱해주면 좌표의 위치가 Z축 기준 45도 회전된다.
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "ZXY축 회전 행렬\n";
	// 각 축마다 특정 각도로 회전 시켜주어야 할 경우 밑의 함수를 사용한다.
	// Yaw는 Y축, Pitch는 X축, Roll는 Z축을 의미하며 함수 삽입 순서도 이와 같다.
	// 반환 값은 Z축 회전행렬 * X축 회전행렬 * Y축 회전행렬이 된다.
	// 이제 이 행렬을 좌표와 곱해주면 좌표의 위치는 z축으로 45도 회전 후, x축으로 30도 회전 후, y축으로 20도 회전하게 된다.
	// 꼭 z -> x -> y 회전 순이라는 것을 명심하자!
	D3DXMatrixRotationYawPitchRoll(&matMatrix, D3DXToRadian(20), D3DXToRadian(30), D3DXToRadian(45));
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "(2, 2, 2)를 축으로 회전 행렬\n";
	D3DXVECTOR3 Axis(2, 2, 2);
	// 밑의 함수는 정해져 있는 X,Y,Z 축이 아닌 임의의 축으로부터 회전 행렬을 제작해준다.
	D3DXMatrixRotationAxis(&matMatrix, &Axis, D3DXToRadian(45)); // 이제 이 행렬을 좌표와 곱해주면 좌표의 위치가 (2, 2, 2)축 기준 45도 회전된다.
	PrintMat(&matMatrix); cout << "\n\n";

	// 이와 같이 이동, 회전, 스케일링 행렬들을 만들 수가 있는데 이를 정점에 적용시킬 때 곱하게 되는 순서가 매우 중요하다.
	// S(scale) * R(rotate) * T(Translate) 의 순서를 꼭 지키지 않으면 엉뚱한 결과가 도출된다.
	// 위 3개 중 Rotate 안에서도 z -> x -> y 축의 회전 순서가 있다. 단 2개의 각이면 순서가 상관 없다. x->y / y->x 상관 없음
}