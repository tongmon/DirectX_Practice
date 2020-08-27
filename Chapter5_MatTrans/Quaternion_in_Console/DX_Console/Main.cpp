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
	D3DXQUATERNION Quaternion; // 쿼터니온은 회전축(x,y,z)과 회전값(Radian)으로 구성된다. 회전 방향은 반시계(CCW)이다.
	D3DXMATRIX matMatrix;

	// 짐벌락 현상을 제거하기 위해 쿼터니온이 사용된다.
	// 이 쿼터니온은 연산 순서가 매트릭스와 다른데 예를 들어 R_Mat = R_Mat_1 * R_Mat_2 라면 R_Mat_1의 회전이 먼저 적용되지만
	// 쿼터니온에서는 Qua = Qua_1 * Qua_2 라면 Qua_2 가 먼저 적용된다.
	// 실사용시에는 쿼터니온 자체를 사용하는 것이 아니라 쿼터니온을 이용해 회전 행렬을 만들어 사용한다.
	// 쿼터니온에 있는 w회전 값은 반시계 회전값을 나타내지만 쿼터니온을 통해 행렬을 만들 때에는 그 행렬에 시계방향 회전값이 들어간다.

	cout << "X축 회전 행렬\n";
	D3DXMatrixRotationX(&matMatrix, D3DXToRadian(45)); // X축 기준 45도 시계방향 회전
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "X축으로 45도 회전하는 값이 저장된 쿼터니온\n";
	D3DXQuaternionRotationMatrix(&Quaternion, &matMatrix); // 회전 행렬을 쿼터니온으로 변환
	printf("회전축 ( %.3f, %.3f, %.3f ) 회전값(호도법): %.3f\n", Quaternion.x, Quaternion.y, Quaternion.z, D3DXToDegree((double)Quaternion.w));
	
	cout << "쿼터니온으로 얻어진 회전 행렬\n";
	D3DXMatrixRotationQuaternion(&matMatrix, &Quaternion); // 구한 쿼터니온을 이제 회전 행렬로 변환하여 얻는 함수, 실사용시 이 행렬이 좌표에 곱해질 것이다.
	PrintMat(&matMatrix); cout << "\n\n";

	D3DXQuaternionRotationYawPitchRoll(&Quaternion, D3DXToRadian(0), D3DXToRadian(45), D3DXToRadian(0)); // 아니면 직접 YawPitchRoll로 제작
	cout << "X축으로 45도 회전하는 값이 저장된 쿼터니온\n";
	printf("회전축 ( %.3f, %.3f, %.3f ) 회전값(호도법): %.3f\n", Quaternion.x, Quaternion.y, Quaternion.z, D3DXToDegree((double)Quaternion.w));
	
	cout << "쿼터니온으로 얻어진 회전 행렬\n";
	D3DXMatrixRotationQuaternion(&matMatrix, &Quaternion);
	PrintMat(&matMatrix); cout << "\n\n";

	D3DXVECTOR3 Axis(1, 0, 0); // X축
	D3DXQuaternionRotationAxis(&Quaternion, &Axis, D3DXToRadian(45));
	cout << "X축으로 45도 회전하는 값이 저장된 쿼터니온\n";
	printf("회전축 ( %.3f, %.3f, %.3f ) 회전값(호도법): %.3f\n", Quaternion.x, Quaternion.y, Quaternion.z, D3DXToDegree((double)Quaternion.w));

	cout << "쿼터니온으로 얻어진 회전 행렬\n";
	D3DXMatrixRotationQuaternion(&matMatrix, &Quaternion);
	PrintMat(&matMatrix); cout << "\n\n";

	cout << "길이가 1인 쿼터니온\n";
	D3DXQuaternionNormalize(&Quaternion, &Quaternion); // 길이가 1인 쿼터니온 얻기
	printf("회전축 ( %.3f, %.3f, %.3f ) 회전값(호도법): %.3f\n", Quaternion.x, Quaternion.y, Quaternion.z, D3DXToDegree((double)Quaternion.w));
}